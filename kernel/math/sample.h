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

#ifndef _SAMPLE_H
#define _SAMPLE_H

#include "math/vec2.h"
#include "math/vec3.h"

inline float SampleSphereCartasian( Vec3 *out, float u, float v )
{
	// sample random direction in local space
	float cost = 1.f - 2 * u;
	float sint = (float) sqrt (1 - cost * cost);
	float phi = (v * 2 - 1) * F_M_PI;
	v3init( out, cos (phi) * sint, cost, sin (phi) * sint );
	
	return 1.f / (4.f * F_M_PI);
}
	
/*
pre:
	u \in [0,1]
	v \in [0,1]
post:
	theta \in [ 0 , PI ]
	phi \in [ -PI , PI ]
*/
inline float SampleSphereSpherical( float *out_theta, float *out_phi, float u, float v )
{
	// sample random direction in local space
	*out_theta = acos(1.f - 2 * u);
	*out_phi = (v * 2 - 1) * F_M_PI;

	return 1.f / (4.f * F_M_PI);
}

inline float SampleHemisphereSolidAngle( Vec3 *out, float u, float v )
{
	// sample random direction in local space
	const float sint = sqrt(1.f - v);
	const float cost = sqrt(v);
	const float phi = 2.f * F_M_PI * u;

	const float x = cos(phi)*sint;
	const float y = sin(phi)*sint;
	const float z = cost;
	
	v3init( out, x, y, z );
	
	return 	cost / F_M_PI;
}

#endif