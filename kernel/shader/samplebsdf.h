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

#ifndef _SAMPLE_BSDF_H
#define _SAMPLE_BSDF_H

#include "math/vec4.h"

#include "shader/fresnel.h"

#include "shader/shader.h"
#include "shader/rgbashader.h"
#include "shader/phongshader.h"
#include "shader/switchshader.h"

#include "shader/shaderenv.h"

#include "shader/channel/evalchannel.h"

// Sample an outgoing direction based on the BSDF. Compute the contribution of this sample per projected solid angle.
// Return true if the outgoing direction is sampled from a singular component of the bsdf.
bool SampleBSDF( Vec3 *refl, Spectrum *bsdf, const Spectrum *spectrum, const Environment *env, const Vec3 *in, const __global char *channels, bool adjoint, Random *rnd )
{
	speczero( bsdf );
	
	// sample random numbers
	float  rnd_brdf = random1f( rnd );
	float2 rnd_refl = random2f( rnd );
	
	float icos = -v3dot (in, &env->norm);
	
	// compute Fresnel term and specular reflection and transmission directions
	Vec3 spec_refl, spec_trans;
	IORRatio fresnel = Fresnel ( &spec_refl, &spec_trans, &env->norm, in, &env->ior );
	
	bool out_specular = false;
	
	// switch on shader type
	switch( env->shader->type )
	{
		case SHADER_RGBA:
		{ 
			__global RGBAShader *rgba = (__global RGBAShader *)env->shader;
		
			Spectrum trans;
			Spectrum diff;
			
			EvalRGBAEnv( &diff, &trans, spectrum, rgba, &fresnel, &env->ior, adjoint );
			
			// compute the probabilities of diffuse reflection and transmission
			// based on color and trans
			float pd = specsum( &diff ) / 3.f;
			float pt = specsum( &trans ) / 3.f;
			float p = pd + pt;
			bool absorbed = p < 1e-7f;
			if (!absorbed)
			{
				p = 1 / p;
				pd *= p;
				pt *= p;
				
				if (rnd_brdf <= pt)
				{
					// this is a transmitted ray
					*refl = spec_trans;
					//float t = trans / pt;
					
					specsmul( bsdf, &trans, 1 /pt );
					//v3init( bsdf, t, t, t );
					
					out_specular = true;
				}
				else
				{
					// sample a perfect diffuse reflection ray using cosine weighted hemisphere sampling
					const float tmp = sqrt(1.f - rnd_refl.y);
					const float tmp2 = 2.f * F_M_PI * rnd_refl.x;

					const float x = cos(tmp2)*tmp;
					const float y = sin(tmp2)*tmp;
					const float z = sqrt(rnd_refl.y);
					
					// compute orthonormal basis
					Vec3 T, B;
					v3perp( &T, &env->norm );
					v3cross( &B, &T, &env->norm );
					
					// transform to world space
					v3smul( refl, &env->norm, z );
					v3smad( refl, &T, x, refl );
					v3smad( refl, &B, y, refl );
					
					// the diffuse BRDF and cosine weighted probability distribution cancel out
					specsmul( bsdf, &diff, 1 / pd );
				}
			}
		}
		break;
		case SHADER_PHONG:
		{
			__global PhongShader *phong = (__global PhongShader *)env->shader;
			
			Spectrum refl_diff, refl_spec;
			Spectrum trns_diff, trns_spec;
				
			EvalPhongEnv(
				&refl_diff, &refl_spec, 
				&trns_diff, &trns_spec,
				spectrum,
				phong,
				env,
				&fresnel, adjoint,
				channels );
										
			// compute probabilities
			float pd = specsum( &refl_diff );
			float ps = specsum( &refl_spec );
			float pdt = specsum( &trns_diff );
			float pst = specsum( &trns_spec );
			float brdf_prob = pd + ps + pdt + pst;
			
			if (brdf_prob > 1e-7f)
			{
				// normalize probabilities
				float invp = 1 / brdf_prob;
								
				pd *= invp; // pd: diffusely reflected fraction
				ps *= invp; // ps: specularly reflected fraction
				pdt *= invp; // pt: diffusely transmitted fraction
				pst *= invp; // pt: specularly transmitted fraction
				// pd, ps, pdt and pst sum up to 1, or all are 0 if ray is completely absorbed
				
				bool transmitted;
				bool specular;
				const Vec3 *refl_norm;

				// select scattering method
				if (rnd_brdf <= pst)
				{
					transmitted = true;
					specular = true;
					refl_norm = &spec_trans;
				}
				else if (rnd_brdf <= pst + ps)
				{
					transmitted = false;
					specular = true;
					refl_norm = &spec_refl;
				}
				else
				{
					transmitted = rnd_brdf <= pst + ps + pdt;
					specular = false;
					refl_norm = &env->norm;
				}
				
				float shininess;
				// get shininess
				EvaluateChannel( &shininess, spectrum, env, (transmitted?phong->transmit:phong->reflect).shininess, channels );
				
				// singular brdf?
				if( specular && shininess == 1.f )
				{
					out_specular = true;
					
					*refl = *refl_norm;
					
					float pselect = (transmitted ? pst : ps);
					specsmul( bsdf , transmitted ? &trns_spec : &refl_spec , 1.f / pselect );
				}
				else
				{
					// convert shininess
					shininess = convertShininess( shininess );
					
					// generate random outgoing direction in reflection space				
					float sint, cost;
					if (specular)
					{
						// choose theta randomly according to the density
						// (n+1) / 2PI * cos(theta)^n 
						cost = (float) pow (rnd_refl.x, 1 / (shininess + 1));
						sint = (float) sqrt (1 - cost * cost);
					}
					else
					{
						// choose theta randomly according to the density cos(theta)/PI
						cost = (float) sqrt (rnd_refl.x);
						sint = (float) sqrt (1 - rnd_refl.x);
						
						// mirror when transmitted
						if (transmitted)
						{					
							cost = -cost;
						}
					}
					
					const float phi = 2.f * F_M_PI * rnd_refl.y;
					
					const float x = cos(phi)*sint;
					const float y = sin(phi)*sint;
					const float z = cost;
						
					// compute a local orthogonal basis with its z-axis pointing along the reflection normal
					Vec3 T, B;
					v3perp( &T, refl_norm );
					v3cross( &B, &T, refl_norm );
					
					// translate reflection to world space
					v3smul( refl, refl_norm, z );
					v3smad( refl, &T, x, refl );
					v3smad( refl, &B, y, refl );
					
					float cosn = v3dot( refl, &env->norm );
					if (transmitted)
					{
						cosn = -cosn;
					}
					
					// direction points to the back-side, ignore ray.
					// This can only happen for Phong shaders. It reflects
					// the fact that the total reflectivity of Phong's
					// reflection model depends on the angle between out
					// and normal: For non-perpendicular rays, an additional
					// fraction is absorbed.
					if( cosn > 0 )
					{
					#if 1
						// only evaluate specular or diffuse component
						if (specular)
						{
							float prob = (transmitted ? pst : ps) * (shininess + 1);
							specsmul( bsdf , transmitted ? &trns_spec : &refl_spec , cosn * (shininess + 2) / (max(cosn, icos) * prob) );
						}
						else
						{
							float prob = (transmitted ? pdt : pd);
							specsmul( bsdf, transmitted ? &trns_diff : &refl_diff , 1 / prob );
						}
					#else
						// evaluate the entire contribution of both specular and diffuse components
					
						// probability densitity of choosing in as diffusely reflected direction
						float prob = (transmitted ? pdt : pd) * cosn * F_M_1_PI;

						// compute cosine with perfect reflection direction
						float cosr = cost;
						if (!specular)
						{
							cosr = v3dot( refl, transmitted?&spec_trans:&spec_refl));
						}
						
						float spec_prob = (transmitted ? pst : ps);
						if ((cosr > 0) && (spec_prob > 0))
						{
							// angle between in and ideally reflected/refracted direction
							// is less than 90 degress. Thus, this ray could
							// have been chosen as a specularly reflected/refracted ray, too
							cosr = shininessPow (cosr, shininess);
							specmul( bsdf , (transmitted ? &trns_spec : &refl_spec) , (shininess + 2) * cosr * F_M_1_2PI / max(cosn, icos) );
							prob += spec_prob * (shininess + 1) * cosr * F_M_1_2PI;
						}
											
						if( prob > 0 )
						{
							// diffuse contribution
							specsmad( bsdf, (transmitted ? &trns_diff : &refl_diff) , F_M_1_PI , bsdf );
							// project and divide by sample probability
							specsmul( bsdf, bsdf, cosn / prob );
						}
					#endif
					}
				}
			}
		}
		break;
	};
	
	return out_specular;
}

#endif