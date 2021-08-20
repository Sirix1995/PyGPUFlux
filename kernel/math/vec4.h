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

#ifndef _VEC4_H
#define _VEC4_H

inline float4* v4zero( float4* v )
{ (*v) = (float4)(0,0,0,0); return v; }

inline float4* v4one( float4* v )
{ (*v) = (float4)(1,1,1,1); return v; }

inline float4* v4init( float4* v , float x , float y , float z , float w )
{ (*v) = (float4)(x,y,z,w); return v; }

inline float4* v4initf4( float4* v , const float4* f4 )
{ (*v) = (*f4); return v; }

inline float4* v4neg( float4* v , const float4 *a )
{ (*v) = -(*a); return v; }

inline float4* v4add( float4* v , const float4 *a , const float4 *b )
{ (*v) = (*a) + (*b); return v; }

inline float4* v4sub( float4* v , const float4 *a , const float4 *b )
{ (*v) = (*a) - (*b); return v; }

inline float4* v4mul( float4* v , const float4 *a , const float4 *b )
{ (*v) = (*a) * (*b); return v; }

inline float4* v4div( float4* v , const float4 *a , const float4 *b )
{ (*v) = (*a) / (*b); return v; }

inline float4* v4smul( float4* v , const float4 *a , float s )
{ (*v) = (*a) * s; return v; }

inline float4* v4mad( float4* v , const float4 *a , const float4 *b , const float4 *c )
{ (*v) = (*a) * (*b) + (*c); return v; }

inline float4* v4smad( float4* v , const float4 *a , float s , const float4 *b )
{ (*v) = (*a) * s + (*b); return v; }

inline float v4dot( const float4 *a , const float4 *b )
{ return (*a).x * (*b).x + (*a).y * (*b).y + (*a).z * (*b).z + (*a).w * (*b).w; }

inline float v4len( const float4 *a )
{ return sqrt( v4dot( a , a ) ); }

inline float v4sqr( const float4 *a )
{ return (*a).x * (*a).x + (*a).y * (*a).y + (*a).z * (*a).z + (*a).w * (*a).w; }

inline float v4sum( const float4 *a )
{ return (*a).x + (*a).y + (*a).z + (*a).w; }

inline float4* v4norm( float4* v , const float4 *a )
{ float rsp = 1.f / v4len( a ); return v4smul( v , a , rsp ); };

inline float4* v4clamp( float4* v , const float4* a )
{
	*v = max( *a, 0.f );
	return v;
}

#endif /* _float4_H */
