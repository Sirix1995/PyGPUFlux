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

#ifndef _AABB_H
#define _AABB_H

#include "math/ray.h"

typedef struct 
{
	float x0,x1,y0,y1,z0,z1;
}AABB;

inline int testRayAABB( const __global AABB *glb_aabb, const float t, const RayAux *aux )
{
	AABB aabb = *glb_aabb;

	// Perform 2 ray-box tests
	const float x0 = aabb.x0 * aux->idir.x - aux->ood.x;
	const float x1 = aabb.x1 * aux->idir.x - aux->ood.x;
	const float y0 = aabb.y0 * aux->idir.y - aux->ood.y;
	const float y1 = aabb.y1 * aux->idir.y - aux->ood.y;
	const float z0 = aabb.z0 * aux->idir.z - aux->ood.z;
	const float z1 = aabb.z1 * aux->idir.z - aux->ood.z;
	float tmin = max(0.f, min(x0, x1));
	float tmax = min(t,max(x0, x1));
	tmin = max(tmin, min(y0, y1));
	tmax = min(tmax, max(y0, y1));
	tmin = max(tmin, min(z0, z1));
	tmax = min(tmax, max(z0, z1));
	
	const int traverse = (tmax >= tmin);
	
	return traverse;
}

#endif