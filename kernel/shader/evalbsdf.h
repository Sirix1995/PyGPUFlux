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

#ifndef _EVAL_BSDF_H
#define _EVAL_BSDF_H

#include "math/vec4.h"

#include "shader/shader.h"
#include "shader/rgbashader.h"
#include "shader/phongshader.h"
#include "shader/switchshader.h"

#include "shader/channel/evalchannel.h"
#include "shader/fresnel.h"
#include "shader/shaderenv.h"

// Evaluate the diffuse term of the BSDF including cosine term, the outgoing direction is assumed not to coincide with the direction of perfect reflection/refraction
//bool EvaluateDiffuseBSDF( Spectrum *bsdf, const Spectrum *spectrum, const Vec3 *p, const Vec3 *lp, const Vec3 *in, const Vec3 *out, const Vec3 *norm, const Vec2 *uv, const IORRatio *ior, const __global Shader* shader, const __global char *channels, bool adjoint )
bool EvaluateDiffuseBSDF( Spectrum *bsdf, const Spectrum *spectrum, const Environment *env, const Vec3 *in, const Vec3 *out, const __global char *channels, bool adjoint )
{	
	float odot = v3dot( out, &env->norm );
	float idot = -v3dot( in, &env->norm );
	
	speczero( bsdf );
		
	IORRatio fresnel = EvalFresnel ( &env->norm, in, &env->ior );
	
	bool reflect = ((idot > 0) == (odot > 0 ));
	bool diffuse = false;
	
	// switch on shader type
	switch( env->shader->type )
	{
		case SHADER_RGBA:
		{
			__global RGBAShader *rgba = (__global RGBAShader *)env->shader;
		
			if( reflect )
			{
				Spectrum trans;
				EvalRGBAEnv( bsdf, &trans, spectrum, rgba, &fresnel, &env->ior, adjoint );
			
				diffuse =  reflect;
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
				
			*bsdf = reflect?refl_diff:trns_diff;
			
			// get shininess
			float shininess;
			EvaluateChannel( &shininess, spectrum, env, (reflect?phong->reflect:phong->transmit).shininess, channels );
						
			// reflect ray
			Vec3 rdir;
			v3smad(	&rdir, &env->norm, -2.f * v3dot( in, &env->norm ), in );
			float s = v3dot( &rdir , out );
			
			if( !reflect )
				s = -s;
			
			if (s > 0)
			{
				// account for non-perfect(glossy) specular reflection
				if( shininess < 1.f )
				{
					// convert shininess
					shininess = convertShininess( shininess );
					
					specsmad( bsdf , reflect?&refl_spec:&trns_spec, shininessPow( s, shininess), bsdf );
				}
			}
						
			diffuse = true;
		}
		break;
	};
		
	specsmul( bsdf, bsdf, fabs(odot) );
	return diffuse;
}

// Evaluate the specular term of the BSDF including cosine term, the outgoing direction is assumed to coincide with the direction of perfect reflection/refraction
//bool EvaluateSpecularBSDF( Spectrum *bsdf, const Spectrum *spectrum, const Vec3 *p, const Vec3 *lp, const Vec3 *in, const Vec3 *out, const Vec3 *norm, const Vec2 *uv, const IORRatio *ior, const __global Shader* shader, const __global char *channels, bool adjoint )
bool EvaluateSpecularBSDF( Spectrum *bsdf, const Spectrum *spectrum, const Environment *env, const Vec3 *in, const Vec3 *out, const __global char *channels, bool adjoint )
{	
	specone( bsdf );
		
	float odot = v3dot( out , &env->norm );
	float idot = -v3dot( in, &env->norm );
	bool reflect = ((idot > 0) == (odot > 0 ));
	
	IORRatio fresnel = EvalFresnel ( &env->norm, in, &env->ior );
	
	bool specular = false;
	
	// switch on shader type
	switch( env->shader->type )
	{
		case SHADER_RGBA:
		{
			__global RGBAShader *rgba = (__global RGBAShader *)env->shader;
			
			if( !reflect )
			{
				Spectrum diff;
				EvalRGBAEnv( &diff, bsdf, spectrum, rgba, &fresnel, &env->ior, adjoint );
				specular = true;
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
				
			// get shininess
			float shininess;
			EvaluateChannel( &shininess, spectrum, env, (reflect?phong->reflect:phong->transmit).shininess, channels );
				
			// account for perfect specular reflection
			if( shininess == 1.f )
			{
				*bsdf = reflect?refl_spec:trns_spec;
				specular = true;
			}
		}
		break;
	};
		
	return specular;
}


#endif
