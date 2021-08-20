
	
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

#ifndef _EVAL_LIGHT_H
#define _EVAL_LIGHT_H

#include "math/math.h"
#include "math/ray.h"
#include "math/environmentmap.h"
#include "math/sample.h"

#include "light/light.h"
#include "light/pointlight.h"
#include "light/spotlight.h"
#include "light/directionallight.h"
#include "light/arealight.h"
#include "light/physicallight.h"
#include "light/environmentlight.h"

#include "shader/evalbsdf.h"
#include "shader/derefshader.h"

/** 
 * evaluate the incomming radiance at a surface point due to an environment map
 * 
 * @param bsdf out:incomming radiance per unit solid angle
 * @param d in:outgoing direction
 * @param p in: surface point
 * @param envlight in: environment map
  */
void EvalEnvironmentLight( Spectrum *bsdf, const Spectrum *spectrum, const Vec3 *d, const Vec3 *p, const __global EnvironmentLight* envlight, const __global char *shaders, const __global char *channels )
{
	const __global Light *light = &envlight->base;
	
	// transform direction to world space
	Vec3 ld;
	TransformLightDirection2World( &ld, light, d );
	
	// get map coordinates
	Vec2 uv;
	CartasianToMap( &uv.x, &uv.y, &ld );
	
	EvalLocalEnvironmentLight( bsdf, spectrum, &ld, p, &uv, envlight, shaders, channels );
			
	// get light power
	Vec3 power = light->power;
	
	Spectrum spectralpower;
	RGB2Spectral( &spectralpower, &power, spectrum );
	
	// multiply with bsdf
	specmul( bsdf, bsdf, &spectralpower );
}

#endif