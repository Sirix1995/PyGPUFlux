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

#ifndef _CAMERA_H
#define _CAMERA_H

#include "math/vec2.h"
#include "math/vec3.h"
#include "math/ray.h"

#define CAMERA_PROJECT  0x1
#define CAMERA_PARALLEL 0x2

typedef struct
{
	int type; // 4
	Vec3 left, up, to, at; // 4 * 3 * 4 = 48
	int width, height; // 4 * 2 = 8
}Camera;

inline Camera* initCamera( Camera* camera, 
	int type, int width, int height,
	float atx, float aty, float atz,
	float tox, float toy, float toz,
	float upx, float upy, float upz	)
{
	camera->type = CAMERA_PROJECT;
	camera->width = width;
	camera->height = height;
	
	Vec3 at,to,up,left;
	v3init( &at, atx, aty, atz );
	v3init( &to, tox, toy, toz );
	v3init( &up, upx, upy, upz );
	
	v3sub( &to, &to, &at );
	v3cross( &left, &to, &up );
	v3cross( &up, &to, &left );
	
	v3norm( &to, &to );
	v3norm( &left, &left );
	v3norm( &up, &up );
	
	camera->at = at;
	camera->to = to;
	camera->up = up;
	camera->left = left;
	
	return camera;
}

inline Ray* initPrimaryRay( Ray *r, __constant Camera *camera , float x , float y )
{
	Vec3 to = camera->to, at = camera->at;
	Vec3 left = camera->left, up = camera->up;
	if( camera->type == CAMERA_PROJECT )
	{
		Vec3 d;
		v3norm( &d , v3add( &d , &to , v3add( &d , v3smul( &left , &left , x ) , v3smul( &up , &up , y ))));
		rinit( r , &at , &d );
	}
	else
	{
		Vec3 o;
		v3add( &o , &at , v3add( &o , v3smul( &left , &left , x ) , v3smul( &up , &up , y )));
		rinit( r , &o , &to );
	}

	return r;
}

inline bool projectOnImage( Vec2 *uv, Vec3 *projp, const Vec3 *p, __constant Camera *camera )
{	
	const Vec3 to = camera->to, at = camera->at;
	const Vec3 left = camera->left, up = camera->up;
	
	Vec3 d;
	v3sub( &d , p , &(at) );
	
	float t = v3dot( &d , &to );
	
	if( t <= 0.f )
		return false;
	
	*projp = at;
	
	if( camera->type == CAMERA_PROJECT )
		v3smul( &d, &d, 1.f / t );
	else
		v3smad( projp, &to, -t, p );
					
	float u = v3dot( &d , &left ) / v3sqr( &left );
	float v = v3dot( &d , &up ) / v3sqr( &up );
	
	if( u < -1.0f || u >= 1.f || v < -1.0f || v >=1.f )
		return false;
	
	v2init( uv, u, v );
	
	return true;
}

#endif
