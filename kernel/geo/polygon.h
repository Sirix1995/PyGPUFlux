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

#ifndef _POLYGON_H
#define _POLYGON_H

#include "geo/prim.h"

typedef struct
{
	Prim base;
	
	Vec3 vert0, vert1, vert2;
	Vec3 n;
	
	Vec2 uv0, uv1, uv2;
	Vec3 nrm0, nrm1, nrm2;
	
}Polygon;

bool computePolygonIntersect( Intc *intc, const Ray *r, const RayAux *aux, const __global Polygon *poly )
{ 
	/*
		Two-sided Ray-triangle intersection test. Published in 'Fast Minimum Storage Ray/Triangle Intersection' by Tomas Moller and Ben Trumbore.
	*/
	
	const Vec3 vert0 = poly->vert0;
	const Vec3 vert1 = poly->vert1;
	const Vec3 vert2 = poly->vert2;
			
	/* find vectors for two edges sharing vert */
	Vec3 edge1, edge2;
	v3sub( &edge1, &vert1, &vert0 );
	v3sub( &edge2, &vert2, &vert0 );
		
	/* begin calculating determinant - also used to calculate U parameter */
	Vec3 pvec;
	v3cross( &pvec, &r->d, &edge2 );
	
	/* if determinant is near zero, ray lies in plane of triangle */
	float det = v3dot( &edge1, &pvec );
	float inv_det = 1.f / det;
	
	/* the non-culling branch */
	if( det > -EPSILON && det < EPSILON )
		return false;
	
	/* calculate distance from vert0 to ray origin */
	Vec3 tvec;
	v3sub( &tvec, &r->o, &vert0 );
	
	/* calculate U parameter and test bounds */
	float u = v3dot( &tvec, &pvec ) * inv_det;
	if( u < 0.f || u > 1.f )
		return false;
		
	/* prepare to test V parameter */
	Vec3 qvec;
	v3cross( &qvec, &tvec, &edge1 );
	
	/* calculate V parameter and test bounds */
	float v = v3dot( &r->d, &qvec ) * inv_det;
	
	if( v < 0 )
		return false;
	
	if( poly->base.type == PRIM_TRIANGLE ) {
		if( u + v > 1.f )
			return false;
	} else {
		if( v > 1.f )
			return false;
	}
	
	/*  calculate t, scale parameters, ray intersects triangle */
	intc->t = v3dot( &edge2, &qvec ) * inv_det;
	v2init( &intc->uv, u, v );
		
	return true; 
}

void computePolygonNormalUV( Vec3 *norm, Vec2 *tex_uv, const Intc *intc, Vec3 *intp, const __global Polygon *poly )
{
	Vec2 uv0 = poly->uv0;
	Vec2 uv1 = poly->uv1;
	Vec2 uv2 = poly->uv2;
	
	Vec2 tri_uv = intc->uv;
	
	v2smul( &uv0, &uv0, 1.f - tri_uv.x - tri_uv.y );
	v2smul( &uv1, &uv1, tri_uv.x );
	v2smul( &uv2, &uv2, tri_uv.y );
		
	v2add( tex_uv, v2add( tex_uv, &uv0, &uv1 ), &uv2 );

#ifdef INTERPOLATE_NORMALS
	// interpolate shading normal
	Vec3 n0 = poly->nrm0;
	Vec3 n1 = poly->nrm1;
	Vec3 n2 = poly->nrm2;
	
	v3smul( &n0, &n0, 1.f - tri_uv.x - tri_uv.y );
	v3smul( &n1, &n1, tri_uv.x );
	v3smul( &n2, &n2, tri_uv.y );
	
	v3add( norm, v3add( norm, &n0, &n1 ), &n2 );
		
	// normalize shading normal
	v3norm( norm , norm );
#else
	*norm = poly->n;
#endif

}

#endif