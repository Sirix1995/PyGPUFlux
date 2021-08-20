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

#ifndef _ENVIRONMENT_LIGHT_H
#define _ENVIRONMENT_LIGHT_H

#include "util/util.h"
#include "math/vec3.h"
#include "math/probability.h"
#include "color/hdrimage.h"

//#define ENVLIGHT_SHADER

typedef struct {
	Light base;
	
	// shader
	int shader_offset;
	
	//// local offset to sky radiance distribution
	//HDRImage colormap;
	int imageOffset;
	
	//environment map sample distribution
	EnvironmentMap map;
} EnvironmentLight;

inline void EvalLocalEnvironmentLight( Spectrum *bsdf, const Spectrum *spectrum, const Vec3 *d, const Vec3 *p, const Vec2 *uv, const __global EnvironmentLight* envlight, const __global char *shaders, const __global char *channels )
{
	#ifdef ENVLIGHT_SHADER
	
		// get shader
		const __global Shader *shader = evalSwitchShader( envlight->shader_offset, shaders, d, d );
		
		// sample shader
		IORRatio ratio = 1.f;
		
		Vec3 lr;
		v3neg( &lr, d );
		EvaluateDiffuseBSDF( bsdf, spectrum, p, d, &lr, &lr, uv, &ratio, shader, channels, false );
		
	#else
		
		// get hdr image
		const __global HDRImage *image = (const __global HDRImage *)(((const __global char *)&(envlight->map)) + envlight->imageOffset);
		
		Vec2 hdr_uv = *uv;
		hdr_uv.y = 1.f-hdr_uv.y;
		
		// sample sky radiance
		SpectralEvaluateHDRImage( bsdf, spectrum, hdr_uv, image); //&envlight->colormap);
	#endif
}

#endif

