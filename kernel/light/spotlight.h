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

#ifndef _SPOT_LIGHT_H
#define _SPOT_LIGHT_H

#include "light/pointlight.h"
#include "math/vec3.h"

typedef struct {
	PointLight base;
	float innerAngle, outerAngle;
} SpotLight;

float GetSpotLightDensity( const __global SpotLight *light, const Vec3 *dir )
{
	float dot = dir->z;
	
	float outer = (float) cos (light->outerAngle);
	
	if (dot <= outer)
	{
		return 0.f;
	}
	
	float inner = (float) cos (light->innerAngle);
	float d = F_M_2PI * (1.f - 0.5f * (inner + outer));
	if (dot >= inner)
	{
		return 1.f / d;
	}
	dot = (dot - outer) / (inner - outer);
	return (3.f - 2.f * dot) * dot * dot / d;
}

#endif

