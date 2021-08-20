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

#ifndef _VEC3_H
#define _VEC3_H

typedef struct
{
	union {
		struct{
			float x,y,z;
		};
		float v[3];
	};
}Vec3;

inline Vec3* v3init( Vec3* v , float x , float y , float z )
{ v->x = x; v->y = y; v->z = z; return v; }

inline Vec3* v3zero( Vec3* v )
{ return v3init( v, 0, 0, 0 ); }

inline Vec3* v3one( Vec3* v )
{ return v3init( v, 1, 1, 1 ); }

inline Vec3* v3initf4( Vec3* v , const float4* f4 )
{ return v3init( v, (*f4).x, (*f4).y, (*f4).z ); }

inline Vec3* v3neg( Vec3* v , const Vec3 *a )
{ return v3init( v, -a->x, -a->y, -a->z); }

inline Vec3* v3add( Vec3* v , const Vec3 *a , const Vec3 *b )
{ return v3init( v, a->x + b->x, a->y + b->y, a->z + b->z); }

inline Vec3* v3sub( Vec3* v , const Vec3 *a , const Vec3 *b )
{ return v3init( v, a->x - b->x, a->y - b->y, a->z - b->z); }

inline Vec3* v3mul( Vec3* v , const Vec3 *a , const Vec3 *b )
{ return v3init( v, a->x * b->x, a->y * b->y, a->z * b->z); }

inline Vec3* v3smul( Vec3* v , const Vec3 *a , float s )
{ return v3init( v, a->x * s, a->y * s, a->z * s); }

inline Vec3* v3mad( Vec3* v , const Vec3 *a , const Vec3 *b , const Vec3 *c )
{ return v3init( v, a->x * b->x + c->x, a->y * b->y + c->y, a->z * b->z + c->z); }

inline Vec3* v3smad( Vec3* v , const Vec3 *a , float s , const Vec3 *b )
{ return v3init( v, a->x * s + b->x, a->y * s + b->y, a->z * s + b->z); }

inline float v3dot( const Vec3 *a , const Vec3 *b )
{ return a->x * b->x + a->y * b->y + a->z * b->z; }

inline float v3len( const Vec3 *a )
{ return sqrt( v3dot( a , a ) ); }

inline float v3sqr( const Vec3 *a )
{ return a->x * a->x + a->y * a->y + a->z * a->z; }

inline float v3sum( const Vec3 *a )
{ return a->x + a->y + a->z; }

inline Vec3* v3norm( Vec3* v , const Vec3 *a )
{ float rsp = 1.f / v3len( a ); return v3smul( v , a , rsp ); };

inline Vec3* v3cross( Vec3* v , const Vec3 *a , const Vec3 *b )
{ return v3init( v, a->y * b->z - a->z * b->y, a->z * b->x - a->x * b->z, a->x * b->y - a->y * b->x); }

inline int v3domaxis( const Vec3* v )
{ return fabs( v->x ) > fabs( v->y ) ? (fabs( v->x ) > fabs( v->z ) ? 0 : 2) : (fabs( v->y ) > fabs( v->z ) ? 1 : 2); }

inline int v3dominis( const Vec3* v )
{ return fabs( v->x ) <= fabs( v->y ) ? (fabs( v->x ) <= fabs( v->z ) ? 0 : 2) : (fabs( v->y ) <= fabs( v->z ) ? 1 : 2); }

inline Vec3* v3clamp( Vec3* v , const Vec3* a )
{
	return v3init( v,
		max( 0.f , a->x ),
		max( 0.f , a->y ),
		max( 0.f , a->z ) );
}

inline Vec3* v3perp( Vec3* v , const Vec3 *a )
{
	int ma = v3dominis( a ); 
	if (ma == 0) 
		return v3init( v , 0, a->z, -a->y ); 
	else if (ma == 1) 
		return v3init( v , a->z, 0, -a->x ); 
	else
		return v3init( v , a->y, -a->x, 0 );
}

inline Vec3* v3trans( Vec3 *out, const Vec3 *v, const Vec3 *x_axis, const Vec3 *y_axis, const Vec3 *z_axis )
{
	float x = v->x, y = v->y, z = v->z;
	v3smul( out , x_axis , x );
	v3smad( out , y_axis , y , out );
	v3smad( out , z_axis , z , out );
	return out;
}

inline void interpolate3(Vec3 *out, const Vec3 *f00, const Vec3 *f01, const Vec3 *f10, const Vec3 *f11, float d, float e)
{
	v3smul( out, f00, (1-d)*(1-e) );
	v3smad( out, f01, (1-d)*e, out);
	v3smad( out, f10, d*(1-e), out);
	v3smad( out, f11,     d*e, out);
}

#endif /* _VEC3_H */
