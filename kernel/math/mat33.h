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

#ifndef _MAT33_H
#define _MAT33_H

#include "math/vec3.h"

typedef struct
{
	union {
		struct{
			float m00, m01, m02;
			float m10, m11, m12;
			float m20, m21, m22;
		};
		float m[9];
	};
}Mat33;

inline Mat33* m33init( Mat33 *m,
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22 )
{ 
	m->m00 = m00; m->m01 = m01; m->m02 = m02;
	m->m10 = m10; m->m11 = m11; m->m12 = m12;
	m->m20 = m20; m->m21 = m21; m->m22 = m22;
	return m; 
}

inline Vec3* m33col( Vec3* v, const Mat33 *m, int c )
{ return v3init( v, m->m[c*3+0], m->m[c*3+1], m->m[c*3+2] ); }

inline Vec3* m33row( Vec3* v, const Mat33 *m, int c )
{ return v3init( v, m->m[c+0], m->m[c+3], m->m[c+6] ); }

inline Vec3* m33vmul( Vec3* out, const Mat33 *m, const Vec3 *v )
{ 
	return v3init( out,
		m->m00 * v->x + m->m01 * v->y + m->m02 * v->z ,
		m->m10 * v->x + m->m11 * v->y + m->m12 * v->z ,
		m->m20 * v->x + m->m21 * v->y + m->m22 * v->z ); 
}

inline Vec3* m33vtransmul( Vec3* out, const Mat33 *m, const Vec3 *v )
{ 
	return v3init( out,
		m->m00 * v->x + m->m10 * v->y + m->m20 * v->z ,
		m->m01 * v->x + m->m11 * v->y + m->m21 * v->z ,
		m->m02 * v->x + m->m12 * v->y + m->m22 * v->z ); 
}

inline Mat33* m33trans( Mat33 *out, const Mat33 *m )
{ 
	return m33init( out,
		m->m00 , m->m10 , m->m20 ,
		m->m01 , m->m11 , m->m21 ,
		m->m02 , m->m12 , m->m22 ); 
}

inline Mat33* m33normal2orthogonal( Mat33 *out, const Vec3 *norm )
{
	// compute orthonormal basis
	Vec3 T, B;
	v3perp( &T, norm );
	v3norm( &T, &T );
	v3cross( &B, &T, norm );
	
	return m33init( out ,
		T.x, B.x, norm->x ,
		T.y, B.y, norm->y ,
		T.z, B.z, norm->z 
		);
}


#endif