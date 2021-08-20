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

#ifndef _IMAGE_CHANNEL_H
#define _IMAGE_CHANNEL_H

#include "shader/channel/channel.h"
#include "math/vec3.h"
#include "math/vec2.h"
#include "color/color.h"
#include "color/image.h"

typedef struct {
	Channel base;
	/*
	int width, height;
	uchar4 data[1];
	*/
	Image image;
} ImageChannel;

inline void EvaluateImageChannel4f( float4 *out, Vec3 p, Vec2 uv, const __global ImageChannel* image )
{
	EvaluateImage( out, uv, &image->image );
}

inline void EvaluateImageChannel3f( Vec3 *out, Vec3 p, Vec2 uv, const __global ImageChannel* image )
{
	float4 color;
	EvaluateImageChannel4f( &color, p, uv, image);
	v3init( out, color.x, color.y, color.z );
}

inline void EvaluateImageChannel1f( float *out, Vec3 p, Vec2 uv, const __global ImageChannel* image )
{
	float4 color;
	EvaluateImageChannel4f( &color, p, uv, image);
	*out = color.w;
}

#endif

