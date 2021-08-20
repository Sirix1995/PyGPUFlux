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

#ifndef _SAMPLE_MAX_BSDF_H
#define _SAMPLE_MAX_BSDF_H

#include "math/vec4.h"

#include "shader/shader.h"
#include "shader/rgbashader.h"
#include "shader/phongshader.h"
#include "shader/switchshader.h"

#include "shader/channel/evalchannel.h"

// Sample the directions of maximum reflection/refraction and compute the corresponding colors
// This method approximates the BSDF with one reflection and one refraction direction.
void SampleMaxBSDF( Vec3 *refl_out, Spectrum *refl_bsdf, Vec3 *trans_out, Spectrum *trans_bsdf, const Spectrum *spectrum, const Environment *env, const Vec3 *in, const __global char *channels, bool adjoint )
{
	speczero( refl_bsdf );
	speczero( trans_bsdf );
	
	IORRatio fresnel = Fresnel ( refl_out, trans_out, &env->norm, in, &env->ior );
	
	// switch on shader type
	switch( env->shader->type )
	{
		case SHADER_RGBA:
		{
			__global RGBAShader *rgba = (__global RGBAShader *)env->shader;
			
			Spectrum diff;
			EvalRGBAEnv( &diff, trans_bsdf, spectrum, rgba, &fresnel, &env->ior, adjoint );
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
				
			*refl_bsdf = refl_spec;
			*trans_bsdf = trns_spec;
		}
		break;
	};
}

#endif