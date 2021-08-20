/*
Copyright (c) 2011 Dietger van Antwerpen (dietger@xs4all.nl)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef _GEN_LIGHT_H
#define _GEN_LIGHT_H

#include "math/math.h"
#include "math/ray.h"
#include "math/environmentmap.h"
#include "math/sample.h"

#include "color/color.h"

#include "light/light.h"
#include "light/pointlight.h"
#include "light/spotlight.h"
#include "light/directionallight.h"
#include "light/arealight.h"
#include "light/physicallight.h"
#include "light/environmentlight.h"
#include "light/spectrallight.h"

#include "shader/evalbsdf.h"

typedef struct
{
	Vec3 p;
	float radius;
}SphereVolume;

void GenerateLight( DEBUG_PAR, Ray *r, Spectrum *lb, Spectrum *spectrum, const __global Light *light, Random *rnd, SphereVolume *bounds, const __global char *shaders, const __global char *channels )
{
	// get random numbers
	float2 rnd_u = random2f( rnd );

	Vec3 p, d;
	// compute density
	// for infinite lights, the density is per unit area
	// for finite lights, the density is per unit lightsource
	float density = 1.f;
	Spectrum spectralp;
	
	specone( &spectralp );
	
#ifdef SPECTRAL
	// perform spectral IS to unit spectrum
	spectralp = ImportanceSampleSpectrum(spectrum, light->spectral_cdf, SPECTRAL_WAVELENGTH_BINS, *spectrum );
#endif

	// correct for the number of spectral channels
	specsmul( &spectralp, &spectralp, (float)SPECTAL_CHANNELS);
	
	// map unit spectrum to spectral range
	MapUnit2Spectrum( spectrum, spectrum );
		
	// compute power
	Spectrum spectralpower;
	if( light->type == LIGHT_SPECTRAL )
	{
		// get spectral light
		const __global SpectralLight *spectrallight = (const __global SpectralLight*)light;
		// evaluate spectrum
		EvalSpectrum( &spectralpower, spectrum, &spectrallight->spectrum );
		
		do 
		{
			// skip spectral light (input light is assumed to follow immidiately)
			spectrallight++;
			// get input light
			light = (const __global Light*)spectrallight;
		}while( light->type == LIGHT_SPECTRAL );

	}else if( light->type != LIGHT_SPECTRAL )
	{
		// get rgb power
		Vec3 power = light->power;
		// convert to approximate spectral representation
		RGB2Spectral( &spectralpower, &power, spectrum );
	}
	
	// compensate for wavelength sampling density
	specdiv(&spectralpower,&spectralpower,&spectralp);
	
	switch( light->type )
	{
		case LIGHT_POINT:
		case LIGHT_SPOT:
		case LIGHT_PHYSICAL:
		{
			__global PointLight* pointlight  = (__global PointLight*)light;
			
			// get light position
			p = pointlight->base.mo2w.t;
			
			switch( light->type )
			{
				case LIGHT_POINT:
				{
					// sample random direction in local space
					float cost = 1.f - 2 * rnd_u.x;
					float sint = (float) sqrt (1 - cost * cost);
					float phi = rnd_u.y * 2 * F_M_PI;
					v3init( &d, cos (phi) * sint, cost, sin (phi) * sint );
				}
				break;
				case LIGHT_SPOT:
				{
					__global SpotLight* spotlight  = (__global SpotLight*)light;
					
					float outer = cos (spotlight->outerAngle);
					float inner = cos (spotlight->innerAngle);
					
					float det = 1 - 0.5f * (inner + outer);
					
					float x = (det * rnd_u.x);
					float cost;
					
					det *= F_M_2PI;
					if (2 * x > inner - outer)
					{
						cost = (x + 0.5f * (inner + outer));
					}
					else
					{
						x *= 2 / (inner - outer);

						// solution of 2 t^3 - t^4 = x
						float t = pow ((x > 0.3f) ? x : 0.5f * x, 1 / 3.f);
						while (true)
						{
							float t2 = t * t;
							float delta = x - t2 * (2 * t - t2);
							if ((-1e-3f < delta) && (delta < 1e-3f))
							{
								cost = (float) ((inner - outer) * t + outer);
								break;
							}
							t += delta / (6 * t2 - 4 * t * t2); 
						}
					}
					float sint = (float) sqrt (1 - cost * cost);
					float phi = rnd_u.y * 2 * F_M_PI;
					v3init( &d, cos (phi) * sint, sin (phi) * sint, cost );
				}
				break;
				case LIGHT_PHYSICAL:
				{
					__global PhysicalLight* physicallight  = (__global PhysicalLight*)light;
									
				#if 0
					// simplified sample random direction in local space for debuging purposes
					density *= invSafe(SampleSphereCartasian( &d , rnd_u.x, rnd_u.y ));
					density *= GetEnvironmentMapDensityFromCartasian( &physicallight->map, &d );
				#else
					// sample random direction in local space
					float theta, phi;
					density *= invSafe(SampleEnvironmentMapSpherical( &theta, &phi, &physicallight->map, &rnd_u ));
					density *= GetEnvironmentMapDensityFromSpherical( &physicallight->map, theta, phi );
					
					//density = 1.f;
										
					SphericalToCartasian( &d, theta, phi );
					//density = SampleEnvironmentMap( &d, &physicallight->map, &rnd_u );
					
					//density *= GetEnvironmentMapDensityFromCartasian( &physicallight->map, &d );
				#endif
													
				}
				break;
			};
			
			// transform direction to world space
			TransformLightDirection2World( &d, light, &d );
		}
		break;
		case LIGHT_DIRECTIONAL:
		{
			//const __global DirectionalLight* dirlight  = (const __global DirectionalLight*)light;

			// sample random point on disc
			float r = sqrt( rnd_u.x ) * bounds->radius;
			float p_phi = rnd_u.y * 2 * F_M_PI;
			
			float x = cos (p_phi) * r;
			float y = sin (p_phi) * r;
			
			v3init( &p , x , y , -bounds->radius );
			
			// get local to world transformation
			Mat33 mo2w = light->mo2w.m33;
			
			// transform light position to world space
			m33vmul( &p , &mo2w , &p );
			v3add( &p, &(bounds->p), &p );
				
			// set direction in world space
			m33row( &d, &mo2w, 2 );
			
			density = (F_M_PI * bounds->radius * bounds->radius);
		}
		break;
		case LIGHT_AREA:
		{
			__global AreaLight* arealight  = (__global AreaLight*)light;
	
			// get extra random numbers
			float2 rnd_v = random2f( rnd );
			
			// sample random point and direction
			
			// get light position
			p = arealight->base.mo2w.t;

			Vec3 u = arealight->u;
			Vec3 v = arealight->v;
						
			// sample random point
			v3smad( &p , &u , 2*rnd_u.x-1 , &p );
			v3smad( &p , &v , rnd_u.y , &p );
	
			float exponent = arealight->exponent;
			//density = (exponent + 2.f);
			float cost = (float) pow (rnd_v.x, 1 / (exponent + 2));
			float sint = (float) sqrt (1 - cost * cost);
			float phi = rnd_v.y * 2 * F_M_PI;
			v3init( &d, 
				cos (phi) * sint,
				sin (phi) * sint, 
				-cost);
			
			Mat33 ortho;
			Vec3 norm = arealight->norm;
			m33normal2orthogonal( &ortho , &norm );
			
			// transform
			m33vmul( &d , &ortho , &d );
			v3norm( &d, &d );
		}
		break;
		case LIGHT_SKY:
		{
			__global EnvironmentLight* envlight  = (__global EnvironmentLight*)light;
			
			// sample random point on disc in local space
			float r = sqrt( rnd_u.x ) * bounds->radius;
			float p_phi = rnd_u.y * 2 * F_M_PI;
			
			float x = cos (p_phi) * r;
			float y = sin (p_phi) * r;
			
			v3init( &p , x , y , -bounds->radius );
					
			// get random numbers
			float2 rnd_v = random2f( rnd );
			
			Vec2 uv;
			
			// importance sample direction in local space
			float theta, phi;
			density *= invSafe(SampleEnvironmentMapSpherical( &theta, &phi, &envlight->map, &rnd_v ));
			SphericalToCartasian( &d, theta, phi );
			
			//density /= SampleSphereCartasian( &d , rnd_v.x, rnd_v.y );
			//CartasianToSpherical( &theta, &phi, &d );
							
			// get spherical mapping
			SphericalToMap( &uv.x, &uv.y, theta, phi );
		
			Spectrum bsdf;
			EvalLocalEnvironmentLight( &bsdf, spectrum, &d, &p, &uv, envlight, shaders, channels );
			
			//density = 1.f;
			//specone( &bsdf );
			
			// correct power
			specmul( &spectralpower, &spectralpower, &bsdf );
			
			// transform direction to world space
			TransformLightDirection2World( &d, light, &d );
			v3norm( &d, &d );
			
			// invert direction
			v3neg( &d, &d );
			
			// create orthagonal basis
			Mat33 ortho;
			m33normal2orthogonal( &ortho , &d );
			
			// transform position to world space
			m33vmul( &p , &ortho , &p );
			v3add( &p, &(bounds->p), &p );
			
			// position density
			density *= (F_M_PI * bounds->radius * bounds->radius);
			
		}
		break;
	};

	specsmul( lb, &spectralpower, density );
	
	rinit( r , &p , &d );
	rmarch( &r->o , 0.001f , r );
}

#endif