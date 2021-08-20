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

#ifndef _BOX_H
#define _BOX_H

#include "geo/prim.h"

typedef struct
{
	Prim base;
}Box;

bool computeBoxIntersect( Intc *intc, const Ray *r, const __global Box *box )
{ 
	const float ooeps = 1.f/pow(2.f,-80.f);
	float idir_x = r->d.x==0.f ? ooeps : 1.f / r->d.x;
	float idir_y = r->d.y==0.f ? ooeps : 1.f / r->d.y;
	float idir_z = r->d.z==0.f ? ooeps : 1.f / r->d.z;

	// Perform ray-box test
	const float ood_x  = r->o.x * idir_x;
	const float ood_y  = r->o.y * idir_y;
	const float ood_z  = r->o.z * idir_z;
	
	const float x0Child0 = -idir_x - ood_x;
	const float x1Child0 =  idir_x - ood_x;
	const float y0Child0 = -idir_y - ood_y;
	const float y1Child0 =  idir_y - ood_y;
	const float z0Child0 = -idir_z - ood_z;
	const float z1Child0 =  idir_z - ood_z;
	
	float tminChild0 = min(x0Child0, x1Child0);
	float tmaxChild0 = max(x0Child0, x1Child0);
	tminChild0 = max(tminChild0, min(y0Child0, y1Child0));
	tmaxChild0 = min(tmaxChild0, max(y0Child0, y1Child0));
	tminChild0 = max(tminChild0, min(z0Child0, z1Child0));
	tmaxChild0 = min(tmaxChild0, max(z0Child0, z1Child0));
	
	// check for intersection
	if(tmaxChild0 >= tminChild0 )
	{
		intc->t = tminChild0>0?tminChild0:tmaxChild0;
		return true;
	}
	
	return false; 
}

#define BOX_X_AXIS 0
#define BOX_Y_AXIS 1
#define BOX_Z_AXIS 2

#define BOX_NEAR 0
#define BOX_FAR 1

#define BOX_LEFT	((BOX_X_AXIS * 2) | BOX_NEAR)
#define BOX_RIGHT	((BOX_X_AXIS * 2) | BOX_FAR)
#define BOX_FRONT	((BOX_Y_AXIS * 2) | BOX_NEAR)
#define BOX_BACK	((BOX_Y_AXIS * 2) | BOX_FAR)
#define BOX_BOTTOM	((BOX_Z_AXIS * 2) | BOX_NEAR)
#define BOX_TOP		((BOX_Z_AXIS * 2) | BOX_FAR)

/*
	NOTE: branch divergence reduces SIMT efficiency on the GPU
	Reduce branching using lookup tables might inprove performance
*/
void computeBoxNormalUV( Vec3 *norm, Vec2 *uv, const Intc *intc, const Vec3 *intp, const __global Box *box )
{
	float u,v; 
	
	const int axis = v3domaxis( intp );
	const int bside = (int)(sign(intp->v[axis]) + 1) / 2;
	const int face = axis * 2 + bside;

	switch (face)
	{
		case BOX_LEFT:
			u = 0.125f * (intp->z + 1);
			v = (1.f / 6) * (intp->y + 3);
			v3init( norm, -1, 0, 0);
			break;
		case BOX_RIGHT:
			u = -0.125f * (intp->z - 5);
			v = (1.f / 6) * (intp->y + 3);
			v3init( norm, 1, 0, 0);
			break;
		case BOX_FRONT:
			
			u = 0.125f * (intp->x + 3);
			v = (1.f / 6) * (intp->z + 1);
			v3init( norm, 0, -1, 0);
			break;
		case BOX_BACK:
			
			u = 0.125f * (intp->x + 3);
			v = (-1.f / 6) * (intp->z - 5);
			v3init( norm, 0, 1, 0);
			break;
		case BOX_BOTTOM:
			
			u = -0.125f * (intp->x - 7);
			v = (1.f / 6) * (intp->y + 3);
			v3init( norm, 0, 0, -1);
			break;
		case BOX_TOP:
			
			u = 0.125f * (intp->x + 3);
			v = (1.f / 6) * (intp->y + 3);
			v3init( norm, 0, 0, 1);
			break;
	}	
	
	v2init( uv, u, v);
}

#endif