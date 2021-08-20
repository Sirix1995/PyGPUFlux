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

#ifndef _INTERSECT_H
#define _INTERSECT_H

#include "geo/prim.h"
#include "geo/plane.h"
#include "geo/sphere.h"
#include "geo/frustum.h"
#include "geo/box.h"
#include "geo/polygon.h"
#include "geo/aabb.h"

bool computePrimitiveIntersect( DEBUG_PAR ,
	Intc *intc ,
	int pidx , 
	const __global char *prims,
	const __global int *offsets,
	const Ray *r,
	const RayAux *aux
	 )
 {
	int offset = offsets[pidx];
		
	const __global Prim *prim = (const __global Prim*)(prims + offset);
	
	#ifdef BIH
		// primitive aabb test	
		if( testRayAABB( &prim->aabb, intc->t, aux ) == 0 )
			return false;
	#endif
	
	Intc prim_intc;
	intc_init( &prim_intc ,0.f ,prim  );
	
	if( (prim->type & PRIM_TRANSFORMABLE) != 0 )
	{
		// transformable primitive
		const __global Prim *tp = (const __global Prim *)prim;

		Ray lr;
		
		//if( (prim->type & PRIM_NOP ) != 0 )
		{
		
			Mat34 m = tp->m;
			// transform ray to object space
			m34rmul( &lr , &m , r );
				
			// force computation
			//prim_intc.t = 1000000.f + v3dot( &lr.o , &lr.d );
		
			//if( (prim->type & PRIM_NOP ) != 0 )
			{
				// perform type-dependant intersection routine
				switch(tp->type & PRIM_NOP_MASK)
				{
				case PRIM_PLANE:
					{
						if( !computePlaneIntersect( &prim_intc, &lr, (const __global Plane*)tp ) )
							return false;
					}
					break;
				case PRIM_SPHERE:
					{
						if( !computeSphereIntersect( &prim_intc, &lr, (const __global Sphere*)tp ) )
							return false;
					}
					break;
				case PRIM_FRUSTUM:
					{
						if( !computeFrustumIntersect( &prim_intc, &lr, (const __global Frustum*)tp ) )
							return false;
					}
					break;
				case PRIM_BOX:
					{
						if( !computeBoxIntersect( &prim_intc, &lr, (const __global Box*)tp ) )
							return false;
					}
					break;
				};
			
				//prim_intc.t += 10000.f;
			}
			
		}
	}
	else
	{
		if( !computePolygonIntersect( &prim_intc, r, aux, (const __global Polygon*)prim ) )
			return false;
	}
	
	// track nearest intersection
	if( prim_intc.t > 0 && prim_intc.t < intc->t )
	{
		*intc = prim_intc;
		return true;
	}
	
	return false;
 }

 // return true iff shadow ray is blocked
bool computeIntersect( DEBUG_PAR ,
	Intc *intc ,
	int start , int num , 
	const __global char *prims,
	const __global int *offsets,
	const Ray *r,
	const RayAux *aux,
	bool shadowRay
	)
{
	for( int pidx = start ; pidx < start + num ; pidx++ )
	{
		if(computePrimitiveIntersect( DEBUG_ARG, intc, pidx, prims, offsets, r, aux ) && shadowRay )
			return true;
	}
	return false;
}

#endif