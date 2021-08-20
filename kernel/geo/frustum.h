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

// TODO: rays parallel to frustum axis intersect with square base and top instead of circular base and top.

#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#define FRUSTUM_CYLINDER 0x1
#define FRUSTUM_TOP_OPEN 0x2
#define FRUSTUM_BASE_OPEN 0x4
#define FRUSTUM_UV_ROTATED 0x8

#define FRUSTUM_SIDE_OPEN 0x2
#define FRUSTUM_TOP 0x4
#define FRUSTUM_BASE 0x8

#include "geo/prim.h"
#include "util/util.h"
#include "math/math.h"

typedef struct
{
	Prim base;
	
	float base_h, top_h, scaleV;
	
	int flag;
}Frustum;

bool computeFrustumIntersect( Intc *intc, const Ray *r, const __global Frustum *frustum )
{ 
	float u0 = -FLT_MAX;
	float u1 =  FLT_MAX;
	int f0 = 0, f1 = 0;
	
	if (r->d.z != 0)
	{
	
		u0 = (frustum->base_h - r->o.z) / r->d.z;
		u1 = (frustum->top_h  - r->o.z) / r->d.z;
		
		f0 = FRUSTUM_BASE | ((frustum->flag & FRUSTUM_BASE_OPEN) ? FRUSTUM_SIDE_OPEN : 0 );
		f1 = FRUSTUM_TOP  | ((frustum->flag & FRUSTUM_TOP_OPEN ) ? FRUSTUM_SIDE_OPEN : 0 );
		
		if( u1 < u0 )
		{
			swapFloat( &u0 , &u1 );
			swapInt( &f0 , &f1 );
		}
	}
	
	float a = r->d.x * r->d.x + r->d.y * r->d.y;
	float b = r->o.x * r->d.x + r->o.y * r->d.y;
	float c = r->o.x * r->o.x + r->o.y * r->o.y;
	
	if(	frustum->flag & FRUSTUM_CYLINDER )
	{
		c -= 1.f;
	}
	else
	{
		a += - r->d.z * r->d.z;
		b += - r->o.z * r->d.z;
		c += - r->o.z * r->o.z;
	}	
	
	float det = b * b - a * c;
	
	if( det < 0.f )
		return false;
		
	det = sqrt(det);
	
	if( a == 0 )
	{
		if( c > 0 )
			return false;
	}
	else
	{
		if (a > 0 || r->d.z > 0 )
		{
			float u = -(det + b) / a;
			if (u > u0)
			{
				u0 = u;
				f0 = 0;
			}
		}
		if (a > 0 || r->d.z < 0 )
		{
			float u = (det - b) / a;
			if (u < u1)
			{
				u1 = u;
				f1 = 0;
			}
		}
	}

	if (u1 < u0)
		return false;
		
	if( u0 > 0.f && (f0 & FRUSTUM_SIDE_OPEN) == 0 )
		intc->t = u0, intc->side = f0;
	else if( u1 > 0.f && (f1 & FRUSTUM_SIDE_OPEN) == 0 )
		intc->t = u1, intc->side = f1;
	else
		return false;
		
	return true;
}

// NOTE: the uv mapping for cylinders is slightly different
void computeFrustumNormalUV( Vec3 *norm, Vec2 *uv, const Intc *intc, const Vec3 *intp, const __global Frustum *frustum )
{
	if( intc->side & FRUSTUM_BASE )
	{
		uv->x = 0.5f * (norm->x / frustum->base_h + 1);
		uv->y = 0.5f * (norm->y / frustum->base_h - 1);	
			
		v3init( norm, 0.f, 0.f, 1.f );
	}
	else if( intc->side & FRUSTUM_TOP  )
	{
		uv->x = 0.5f * (norm->x / frustum->top_h + 1);
		uv->y = 0.5f * (1 - norm->y /frustum->top_h);
				
		v3init( norm, 0.f, 0.f, -1.f );
	}
	else
	{
		uv->x = (atan2 (norm->y, -norm->x) + F_M_PI) * (1 / (2 * F_M_PI));
		
		if(	frustum->flag & FRUSTUM_CYLINDER )
		{
			uv->y = 0.5f * (norm->z + 1);
			
			norm->z = 0.f;
		}
		else
		{
			uv->y = (norm->z - frustum->top_h) / (frustum->base_h - frustum->top_h);
			
			if (frustum->flag & FRUSTUM_UV_ROTATED)
				uv->x = 1 - uv->x;
			else
				uv->y = 1 - uv->y;
				
			norm->z = -intp->z;
		}
		
		uv->y *= frustum->scaleV;
		
		norm->x = intp->x;
		norm->y = intp->y;
	}
}

#endif