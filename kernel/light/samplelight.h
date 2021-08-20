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

#ifndef _SAMPLE_LIGHT_H
#define _SAMPLE_LIGHT_H

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
#include "shader/derefshader.h"


/** 
 * sample the incomming radiance at a surface point due to a light source in view
 * 
 * @param r out: ray from surface point to light source
 * @param dist out:distance to light source
 * @param lb out:incomming radiance per unit solid angle
 * @param p in: surface point
 * @param light in: light source
 * @param rnd in: random number generator
  */
void SampleLight( Ray *r, float *dist, Spectrum *lb, const Spectrum *spectrum, const Vec3 *p, const __global Light *light, Random *rnd, const __global char *shaders, const __global char *channels )
{
	// get random numbers
	float2 rnd_u = random2f( rnd );

	Vec3 ld;
	float length;
	
	// compute density
	// for infinite lights, the density is per unit area
	// for finite lights, the density is per unit lightsource
	float density = 1.f;
	
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
	}
	else
	{
		// get rgb power
		Vec3 power = light->power;
		// convert to approximate spectral representation
		RGB2Spectral( &spectralpower, &power, spectrum );
	}
	
	switch( light->type )
	{
		case LIGHT_POINT:
		{
			const __global PointLight* pointlight  = (const __global PointLight*)light;
			
			// get light position
			Vec3 lp = pointlight->base.mo2w.t;
			
			// construct light direction
			v3sub( &ld, &lp, p );
			length = v3len( &ld );
			v3smul( &ld , &ld , 1.f / length );
			
			// compute density
			density = F_M_1_2PI / (2.f);
			
			// convert volume to solid angle
			density	*= invSafe(length*length);
		}
		break;
		case LIGHT_DIRECTIONAL:
		{
			const __global DirectionalLight* dirlight  = (const __global DirectionalLight*)light;
			
			// construct light direction
			Mat34 mw2o = dirlight->base.mo2w;
			m34row( &ld, &mw2o, 2 );
			v3neg(  &ld,  &ld );
			length = FLT_MAX;
			
			// compute density
			density = 1.f;
		}
		break;
		case LIGHT_SPOT:
		{
			const __global SpotLight* spotlight  = (const __global SpotLight*)light;
			
			// get light position
			Vec3 lp = spotlight->base.base.mo2w.t;
			
			// construct light direction
			v3sub( &ld, &lp, p );
			length = v3len( &ld );
			v3smul( &ld , &ld , 1.f / length );
			
			// transform light direction to local space
			Vec3 old;
			TransformLightDirection2Object( &old , light, &ld );
			v3neg( &old, &old );

			// compute density
			density = GetSpotLightDensity( spotlight, &old );
		
			// convert volume to solid angle
			density	*= invSafe(length*length);
		}
		break;
		case LIGHT_PHYSICAL:
		{
			const __global PhysicalLight* physicallight  = (const __global PhysicalLight*)light;
			
			// get light position
			Vec3 lp = physicallight->base.base.mo2w.t;
			
			// construct light direction
			v3sub( &ld, &lp, p );
			length = v3len( &ld );
			v3smul( &ld , &ld , 1.f / length );
			
			// transform light direction to local space
			Vec3 old;
			TransformLightDirection2Object( &old , light, &ld );
			v3neg( &old, &old );
			
			// compute density
			density *= GetEnvironmentMapDensityFromCartasian( &physicallight->map, &old );
			
			// convert volume to solid angle
			density	*= invSafe(length*length);
		}
		break;
		case LIGHT_AREA:
		{
			const __global AreaLight* arealight  = (const __global AreaLight*)light;
	
			// get random numbers
			float2 rnd_u = random2f( rnd );
			
			// sample point on area light
			Vec3 lp = arealight->base.mo2w.t;
			
			Vec3 u = arealight->u;
			Vec3 v = arealight->v;
						
			v3smad( &lp , &u , 2*rnd_u.x-1 , &lp );
			v3smad( &lp , &v , rnd_u.y , &lp );
			
			// construct light direction
			v3sub( &ld, &lp, p );
			length = v3len( &ld );
			v3smul( &ld , &ld , 1.f / length );
			
			Vec3 n = arealight->norm;
			
			float b = v3dot( &n, &ld );
			
			// compute density
			density = 0.f;
			
			if( b > 0.f )
			{
				density = (arealight->exponent + 2.f) * (float) pow (b , arealight->exponent + 1.f) * F_M_1_2PI;
				
				// convert area to solid angle
				density	*= b * invSafe(length*length);
			}
		}
		break;
		case LIGHT_SKY:
		{
			const __global EnvironmentLight* envlight  = (const __global EnvironmentLight*)light;
			
			// get random numbers
			float2 rnd_u = random2f( rnd );
			
			Vec2 uv;
			// sample direction in local space
			float theta, phi;
			density *= invSafe(SampleEnvironmentMapSpherical( &theta, &phi, &envlight->map, &rnd_u ));
				
			// transform direction to cartasian coordinates
			SphericalToCartasian( &ld, theta, phi );
			
			// get spherical mapping
			SphericalToMap( &uv.x, &uv.y, theta, phi );
			
			Spectrum bsdf;		
			EvalLocalEnvironmentLight( &bsdf, spectrum, &ld, p, &uv, envlight, shaders, channels );
			
			// correct power
			specmul( &spectralpower, &spectralpower, &bsdf );
			
			// transform direction to world space
			TransformLightDirection2World( &ld, light, &ld );
			v3norm( &ld, &ld );
			
			// set length
			length = FLT_MAX;
		}
		break;
	};

	specsmul( lb, &spectralpower, density );
	
	rinit( r , p , &ld );
	rmarch( &r->o , CONNECT_EPSILON , r );
	
	*dist = length - 2*CONNECT_EPSILON;
}

#endif