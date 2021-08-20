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

#ifndef _PHONGSHADER_H
#define _PHONGSHADER_H

#include "shader/shader.h"
#include "shader/shaderenv.h"
#include "shader/channel/evalcolorchannel.h"

typedef struct
{
	ChannelHandle diffuse;
	ChannelHandle specular;
	ChannelHandle shininess;
} PhongSide;

typedef struct
{
	Shader base;
	
	PhongSide reflect, transmit;
	bool interpolatedTransparency;
} PhongShader;

inline float shininessPow(float x, float shininess)
{
	return (x >= 1) ? 1 : (float) pow (x, shininess);
}

inline float convertShininess (float x)
{
	x = x * (2 - x);
	if (x <= 0)
	{
		return 0;
	}
	else if (x >= 1)
	{
		return FLT_MAX / 2;
	}
	else
	{
		return min (-2 / (float) log (x), FLT_MAX / 2);
		}
}

inline void EvalPhongEnv(
	Spectrum *refl_diff, Spectrum *refl_spec, 
	Spectrum *trns_diff, Spectrum *trns_spec,
	const Spectrum *spectrum,
	const __global PhongShader *phong,
	const Environment *env,
	const IORRatio *fresnel, bool adjoint,
	const __global char *channels )
{
	float alpha;
	// evaluate alpha-color channels
	//EvaluateAlphaColorChannel( &alpha, refl_diff, spectrum, env->p, env->uv, phong->reflect.diffuse , channels );
	EvaluateAlphaColorChannel( &alpha, refl_diff, spectrum, env, phong->reflect.diffuse , channels );
				
	// evaluate color channels
	//EvaluateColorChannel( refl_spec, spectrum, env->p, env->uv, phong->reflect.specular, channels );
	//EvaluateColorChannel( trns_diff, spectrum, env->p, env->uv, phong->transmit.diffuse , channels );
	//EvaluateColorChannel( trns_spec, spectrum, env->p, env->uv, phong->transmit.specular, channels );
	EvaluateColorChannel( refl_spec, spectrum, env, phong->reflect.specular, channels );
	EvaluateColorChannel( trns_diff, spectrum, env, phong->transmit.diffuse , channels );
	EvaluateColorChannel( trns_spec, spectrum, env, phong->transmit.specular, channels );
	
	// apply diffuse alpha channel
	(*trns_spec) = 1 + alpha * (*trns_spec - 1);
	//trns_spec->x = 1 + alpha * (trns_spec->x - 1);
	//trns_spec->y = 1 + alpha * (trns_spec->y - 1);
	//trns_spec->z = 1 + alpha * (trns_spec->z - 1);
	
	// interpolate transparancy
	if (phong->interpolatedTransparency)
	{
		(*refl_diff) *= 1 - (*trns_spec);
		//refl_diff->x *= 1 - trns_spec->x;
		//refl_diff->y *= 1 - trns_spec->y;
		//refl_diff->z *= 1 - trns_spec->z;
	}
	
	if( isChannel( phong->reflect.specular ) )
	{
		// interpolate transparancy
		if (phong->interpolatedTransparency)
		{	
			(*refl_spec) *= 1 - (*trns_spec);
			//refl_spec->x *= 1 - trns_spec->x;
			//refl_spec->y *= 1 - trns_spec->y;
			//refl_spec->z *= 1 - trns_spec->z;
		}

		// increase reflection by reflected transparancy (due to fresnel)
		(*refl_spec) = (*trns_spec) * (*fresnel) + (*refl_spec);
		//v3smad( refl_spec, trns_spec, fresnel, refl_spec );
	}
	
	// decrease transparancy by inverse fresnel term
	// correct for adjoint sampling
	if (adjoint)
	{
		(*trns_spec) *= 1 - (*fresnel);
		//v3smul( trns_spec, trns_spec, 1 - fresnel );
	}
	else
	{
#ifdef REFRACTION
		(*trns_spec) *= ((env->ior) * (env->ior) * (1 - (*fresnel)));
		(*trns_diff) *= ((env->ior) * (env->ior));
#else
		(*trns_spec) *= ((1 - (*fresnel)));
		(*trns_diff) *= (1);
#endif
		//v3smul( trns_spec, trns_spec, ior * ior * (1 - fresnel));
		//v3smul( trns_diff, trns_diff, ior * ior);
	}
}

#endif
