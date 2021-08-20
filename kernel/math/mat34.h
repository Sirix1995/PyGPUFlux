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

#ifndef _MAT34_H
#define _MAT34_H

#include "math/mat33.h"

typedef struct
{
	union {
		struct{
			Mat33 m33;
			Vec3 t;
		};
		float m[12];
	};
}Mat34;

inline Mat34* m34init( Mat34 *m,
	float m00, float m01, float m02,
	float m10, float m11, float m12,
	float m20, float m21, float m22,
	float m30, float m31, float m32	)
{ 
	m33init( &m->m33 ,
		m00, m01, m02,
		m10, m11, m12,
		m20, m21, m22 );
	v3init( &m->t, m30, m31, m32 );
	return m; 
}

inline Vec3* m34col( Vec3* v, const Mat34 *m, int c )
{ return v3init( v, m->m[c*3+0], m->m[c*3+1], m->m[c*3+2] ); }

inline Vec3* m34row( Vec3* v, const Mat34 *m, int c )
{ return v3init( v, m->m[c+0], m->m[c+3], m->m[c+6] ); }

inline Vec3* m34vmul( Vec3* out, const Mat34 *m, const Vec3 *v )
{ return m33vmul( out, &m->m33, v ); }

inline Vec3* m34pmul( Vec3* out, const Mat34 *m, const Vec3 *v )
{ 
	m33vmul( out, &m->m33 , v );
	return v3add(  out, out, &m->t ); 
}

inline Vec3* m34submul( Vec3* out, const Mat34 *m, const Vec3 *v )
{ 
	v3sub( out, v, &m->t ); 
	return m33vmul( out, &m->m33, out );
}

inline Vec3* m34vtransmul( Vec3* out, const Mat34 *m, const Vec3 *v )
{ return m33vtransmul( out, &m->m33 , v ); }



#endif