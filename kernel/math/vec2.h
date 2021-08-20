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

#ifndef _VEC2_H
#define _VEC2_H

typedef struct
{
	union {
		struct{
			float x,y;
		};
		float v[2];
	};
}Vec2;

inline Vec2* v2init( Vec2 *v, float x , float y )
{ v->x = x; v->y = y; return v; }

inline Vec2* v2neg( Vec2 *v, const Vec2 *a )
{ return v2init( v, -a->x, -a->y); }

inline Vec2* v2add( Vec2 *v, const Vec2 *a , const Vec2 *b )
{ return v2init( v, a->x + b->x, a->y + b->y); }

inline Vec2* v2sub( Vec2 *v, const Vec2 *a , const Vec2 *b )
{ return v2init( v, a->x - b->x, a->y - b->y); }

inline Vec2* v2mul( Vec2 *v, const Vec2 *a , const Vec2 *b )
{ return v2init( v, a->x * b->x, a->y * b->y); }

inline Vec2* v2smul( Vec2 *v, const Vec2 *a , float s )
{ return v2init( v, a->x * s, a->y * s); }

inline float v2dot( const Vec2 *a , const Vec2 *b )
{ return a->x * b->x + a->y * b->y; }

inline float v2len( const Vec2 *a )
{ return sqrt( v2dot( a , a ) ); }

inline Vec2* v2norm( Vec2 *v, const Vec2 *a )
{ float rsp = 1.f / v2len( a ); return v2smul( v, a , rsp ); };

#endif /* _VEC2_H */
