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

#ifndef _NORMALUV
#define _NORMALUV

#include "geo/prim.h"
#include "geo/plane.h"
#include "geo/sphere.h"
#include "geo/frustum.h"
#include "geo/box.h"
#include "geo/polygon.h"

void computePrimitiveNormalUV( DEBUG_PAR ,
	Vec3 *norm,
	Vec2 *uv,
	const Intc *intc,
	const Vec3 *intp,
	const __global Prim *prim
	 )
{
	if( (prim->type & PRIM_TRANSFORMABLE) != 0 )
	{
		// transformable primitive
		const __global Prim *tp = (const __global Prim *)prim;
			
		// get global to local transformation
		Mat34 m = tp->m;
		
		// transform intersection
		m34submul( norm , &m , intp );
		
		// perform type-dependant compute normal routine
		switch(tp->type)
		{
		case PRIM_PLANE:
			{
				computePlaneNormalUV( norm, uv, intc, norm, (__global Plane*)tp );
			}
			break;
		case PRIM_SPHERE:
			{
				computeSphereNormalUV( norm, uv, intc, norm, (__global Sphere*)tp );
			}
			break;
		case PRIM_FRUSTUM:
			{
				computeFrustumNormalUV( norm, uv, intc, norm, (__global Frustum*)tp );
			}
			break;
		case PRIM_BOX:
			{
				computeBoxNormalUV( norm, uv, intc, norm, (__global Box*)tp );
			}
			break;
		};
		
		// transform normal to global space
		m34vtransmul( norm , &m , norm );
		
		// normalize normal
		v3norm( norm , norm );
	}
	else
	{
		computePolygonNormalUV( norm, uv, intc, norm, (__global Polygon*)prim );
	}
}

#endif