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

#ifndef _SPHERE_H
#define _SPHERE_H

#include "geo/prim.h"

typedef struct
{
	Prim base;
}Sphere;

bool computeSphereIntersect( Intc *intc, const Ray *r, const __global Sphere *sphere )
{
	// equation of sphere
	float a = v3dot( &r->d , &r->d );
	float b = v3dot( &r->o , &r->d );
	float c = v3dot( &r->o , &r->o ) - 1.f;
	
	// equation to solve is: a u^2 + 2bu + c = 0
	float det = b * b - a * c;
	
	if (det <= 0)
		return false;
						
	det = sqrt(det);
						
	// find the nearest intersection
	intc->t = -(det + b);
	if( intc->t < 0 )
		intc->t = (det - b);
	intc->t /= a;
	
	return true;
}
	
void computeSphereNormalUV( Vec3 *norm, Vec2 *uv, const Intc *intc, const Vec3 *intp, const __global Sphere *sphere )
{
	// for a sphere, the local point equals the normal vector
				
	float cosv = norm->z;
	float t = 1 - norm->z * norm->z;
	float sinv = (t <= 0) ? 0 : sqrt (t);

	float cosu;
	float sinu;

	if (sinv == 0)
	{
		cosu = 1;
		sinu = 0;
	}
	else
	{
		cosu = norm->x / sinv;
		sinu = norm->y / sinv;
	}

	float u = atan2 (sinu, cosu) * (1 / (2 * F_M_PI));
	if (u < 0)
	{
		u += 1;
	}
	v2init( uv, u, acos (-cosv) * (1 / F_M_PI));
}
	
#endif