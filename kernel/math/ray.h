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

#ifndef _RAY_H
#define _RAY_H

#include "math/mat34.h"

typedef struct
{
	union
	{
		struct
		{
			Vec3 o, d;
		};
	//	struct
		//{
		//	float _o[3], _d[3];
		//};
	};
}Ray;

typedef struct
{
	union
	{
		struct
		{
			Vec3 ood, idir;
		};
	//	struct
	//	{
	//		float _ood[3], _idir[3];
	//	};
	};
}RayAux;

inline RayAux* raux( RayAux* out, const Ray* r )
{
	const float ooeps = 1.f/pow(2.f,-80.f);
	out->idir.x  = r->d.x==0.f ? ooeps : 1/r->d.x;	// avoid div by 0
	out->idir.y  = r->d.y==0.f ? ooeps : 1/r->d.y;
	out->idir.z  = r->d.z==0.f ? ooeps : 1/r->d.z;
	out->ood.x  = r->o.x * out->idir.x;
	out->ood.y  = r->o.y * out->idir.y;
	out->ood.z  = r->o.z * out->idir.z;
	return out;
}

inline Ray* rinit( Ray* out, const Vec3 *o, const Vec3 *d )
{ out->o = *o; out->d = *d; return out; }

inline Ray* m34rmul( Ray* out, const Mat34 *m , const Ray* in )
{
	/*
	v3sub( &out->o, &in->o, &m->t ); 
	
	v3init( &out->o,
		m->m33.m00 * out->o.x ,
		m->m33.m11 * out->o.y ,
		m->m33.m22 * out->o.z );
	
	v3init( &out->d,
		m->m33.m00 * in->d.x ,
		m->m33.m11 * in->d.y ,
		m->m33.m22 * in->d.z );
	*/
	m34submul( &out->o , m , &in->o );
	//out->d = in->d;
	m33vmul( &out->d , &m->m33 , &in->d );
	return out;
}

inline Vec3* rmarch( Vec3 *o, float t, const Ray* r )
{ return v3smad( o , &r->d , t , &r->o ); };

#endif