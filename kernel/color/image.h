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

#ifndef _IMAGE_H
#define _IMAGE_H

#include "math/vec3.h"
#include "math/vec2.h"
#include "color/color.h"

typedef struct {
	int width, height;
	uchar4 data[1];
} Image;

inline void EvaluateImage( float4 *out, Vec2 uv, const __global Image* image )
{
	int width = image->width;
	int height = image->height;
	
	// convert uv to corresponding positive coordinates
	uv.y = -uv.y;
	if( uv.x < 0.f ) uv.x = ( ceil(-uv.x) ) + uv.x;
	if( uv.y < 0.f ) uv.y = ( ceil(-uv.y) ) + uv.y;
	
	// scale to image resolution
	uv.x *= width;
	uv.y *= height;
	
	// get upper-left pixel
	int x0 = (int)(uv.x);
	int y0 = (int)(uv.y);
	
	// clamp texel coordinates (floating point to integer for very large texture coordinates can cause negative texel coordinates due to overflow)
	if( x0 < 0 ) x0 = 0;
	if( y0 < 0 ) y0 = 0;
	
	// get relative position in pixel
	uv.x -= x0;
	uv.y -= y0;
	
	// repeat
	x0 %= width;
	y0 %= height;
	
	// get lower-right pixel
	int x1 = (x0 + 1) % width;
	int y1 = (y0 + 1) % height;
	
	// clamp texel coordinates (floating point to integer for very large texture coordinates can cause out-of-range texel coordinates due to overflow)
	uv.x = min( 1.f , max( 0.f, uv.x ));
	uv.y = min( 1.f , max( 0.f, uv.y ));
	
	// get filter weights
	const float w00 = (1.f - uv.x) * (1.f - uv.y);
	const float w01 = (1.f - uv.x) *       (uv.y);
	const float w10 = 		(uv.x) * (1.f - uv.y);
	const float w11 =       (uv.x) *       (uv.y);
	
	// get texels
	const int i00 = x0 + y0 * width;
	const int i01 = x0 + y1 * width;
	const int i10 = x1 + y0 * width;
	const int i11 = x1 + y1 * width;
	
	// get bilinear filtered color
	float4 color =	 
		convert_float4( image->data[i00] ) * w00 +
		convert_float4( image->data[i01] ) * w01 +
		convert_float4( image->data[i10] ) * w10 +
		convert_float4( image->data[i11] ) * w11;
					
	// map from color range
	color *= (1.f / 255.f);
	 
	// shuffle color
	*out = (float4)( color.z , color.y , color.x , color.w );
}

#endif

