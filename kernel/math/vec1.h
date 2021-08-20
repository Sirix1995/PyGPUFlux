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

#ifndef _VEC1_H
#define _VEC1_H

inline float* v1zero( float* v )
{ (*v) = 0; return v; }

inline float* v1one( float* v )
{ (*v) = 1; return v; }

inline float* v1init( float *v, float x )
{ *v = x; return v; }

inline float* v1neg( float *v, const float *a )
{ (*v) = -(*a); return v; }

inline float* v1add( float *v, const float *a , const float *b )
{ (*v) = (*a) + (*b); return v; }

inline float* v1sub( float *v, const float *a , const float *b )
{ (*v) = (*a) - (*b); return v; }

inline float* v1mul( float *v, const float *a , const float *b )
{ (*v) = (*a) * (*b); return v; }

inline float* v1div( float *v, const float *a , const float *b )
{ (*v) = (*a) / (*b); return v; }

inline float* v1smul( float *v, const float *a , float s )
{ (*v) = (*a) * s; return v; }

inline float* v1mad( float* v , const float *a , const float *b , const float *c )
{ (*v) = (*a) * (*b) + (*c); return v; }

inline float* v1smad( float* v , const float *a , float s , const float *b )
{ (*v) = (*a) * s + (*b); return v; }

inline float v1dot( const float *a , const float *b )
{ return (*a) * (*b); }

inline float v1sum( const float *a )
{ return (*a); }

inline float* v1clamp( float* v , const float* a )
{
	*v = max( *a, 0.f );
	return v;
}


#endif /* _VEC2_H */
