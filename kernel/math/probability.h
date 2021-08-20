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

#ifndef _PROBABILITY_H
#define _PROBABILITY_H

#define FOCALDIST 1.f

#include "common/camera.h"

// ------------------------------------------------------------------------------------------------- //
// ------------------------------ RELATION BETWEEN UNITS ------------------------------ //
// ------------------------------------------------------------------------------------------------- //

/** 
 * computes the differential relation from unit solid angle to unit surface area
 * 
 * @param p reference point for surface area
 * @param v reference point for solid angle
 * @param n surface area normal
 * @return area around p covered by unit solid angle subtending from v
 */
float SolidAngleToArea( const Vec3 *p, const Vec3 *v, const Vec3 *n )
{
	Vec3 l;
	
	v3sub( &l, v, p );
		
	float dist = v3len( &l );
	v3smul( &l, &l, 1.f / dist );
			
	float dot = fabs(v3dot( &l, n ));

	return (dist*dist) / dot;
}

/** 
 * computes the differential relation from unit surface area to unit solid angle
 * 
 * @param p reference point for surface area
 * @param v reference point for solid angle
 * @param n surface area normal
 * @return solid angle subtending from v covered by unit area around p
 */
inline float AreaToSolidAngle( const Vec3 *p, const Vec3 *v, const Vec3 *n )
{ return 1.f / SolidAngleToArea( p, v, n ); }

/** 
 * computes the differential relation from unit solid angle to unit projected solid angle
 * 
 * @param v direction solid angle is subtended around 
 * @param n surface normal
 * @return projected solid angle subtending by v covered by unit solid angle around v
 */
inline float SolidAngleToProjectedSolidAngle( const Vec3 *v, const Vec3 *n )
{ return v3dot( v, n ); }

/** 
 * computes the differential relation from unit surface area to unit image plane
 * 
 * @param p reference point for surface area
 * @param n normal at reference point
 * @param camera defines the image plane
 * @return unit image space covered by unit area around v
 */
inline float AreaToImage( const Vec3 *p, const Vec3 *n, __constant Camera *camera )
{ 
	float factor = 1.f;

	Vec3 o = camera->at;
	Vec3 eye = camera->to;
	
	if( camera->type == CAMERA_PROJECT )
	{
		Vec3 l;
		
		v3sub( &l , p, &o );
		
		float length = v3len( &l );
		float dot0 = fabs(v3dot( &l, &eye ));
		float dot1 = fabs(v3dot( &l, n ));
		
		factor = (dot1) / (dot0*dot0*dot0);
	}
	else
	{
		float dot = fabs(v3dot( &eye, n ));
		
		factor = dot;
	}

	return factor;
}

/** 
 * computes the differential relation from unit image plane to unit surface area
 * 
 * @param v reference point for solid angle
 * @param n normal at reference point
 * @param camera defines the image plane
 * @return unit area around v covered by unit image space
 */
inline float ImageToArea( const Vec3 *p, const Vec3 *n, __constant Camera *camera )
{ 
	float factor = 1.f;

	Vec3 o = camera->at;
	Vec3 eye = camera->to;
	
	if( camera->type == CAMERA_PROJECT )
	{
		Vec3 l;
		
		v3sub( &l , p, &o );
		
		float length = v3len( &l );
		float dot0 = fabs(v3dot( &l, &eye ));
		float dot1 = fabs(v3dot( &l, n ));
		
		factor = (dot0*dot0*dot0) / (dot1);
	}
	else
	{
		float dot = fabs(v3dot( &eye, n ));
		
		factor = 1.f / dot;
	}
	
	return factor;
}

// ------------------------------------------------------------------------------------------------------------ //
// ------------------------------ RELATION BETWEEN PROBABILITIES ------------------------------ //
// ------------------------------------------------------------------------------------------------------------ //

/** 
 * computes the relation from probability per unit solid angle to probability per unit image plane
 * 
 * @param p reference point for surface area
 * @param v reference point for solid angle
 * @param n normal at reference point
 * @return probability per unit image space for unit probability per unit area around v
 */
inline float PrbSolidAngleToArea( const Vec3 *p, const Vec3 *v, const Vec3 *n )
{ return AreaToSolidAngle( p, v, n ); }

/** 
 * computes the differential relation from probability per unit surface area to probability per unit solid angle
 * 
 * @param p reference point for surface area
 * @param v reference point for solid angle
 * @param n surface area normal
 * @return probability per unit solid angle subtending from v for unit probability per unit area around p
 */
inline float PrbAreaToSolidAngle( const Vec3 *p, const Vec3 *v, const Vec3 *n )
{ return SolidAngleToArea( p, v, n ); }

/** 
 * computes the differential relation from probability per unit surface area to probability per unit image plane
 * 
 * @param v reference point for solid angle
 * @param n normal at reference point
 * @param camera defines the image plane
 * @return probability per unit area around v for unit probability per unit image space
 */
inline float PrbAreaToImage( const Vec3 *v, const Vec3 *n, __constant Camera *camera )
{ return ImageToArea( v, n, camera ); }

/** 
 * computes the differential relation from probability per unit image plane to probability per unit surface area
 * 
 * @param v reference point for solid angle
 * @param n normal at reference point
 * @param camera defines the image plane
 * @return probability per unit area around v for unit probability per unit image space
 */
inline float PrbImageToArea( const Vec3 *v, const Vec3 *n, __constant Camera *camera )
{ return AreaToImage( v, n, camera ); }

#endif
