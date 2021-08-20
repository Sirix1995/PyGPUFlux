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

#ifndef _COLOR_CHANNEL_H
#define _COLOR_CHANNEL_H

#include "math/vec3.h"
#include "shader/channel/channel.h"

typedef struct {
	Channel base;
	Vec3 rgb;
} RGBChannel;

inline void EvaluateRGBChannel4f( float4 *out, Vec3 p, Vec2 uv, const __global RGBChannel* color )
{
	*out = (float4)( color->rgb.x, color->rgb.y, color->rgb.z, 1.f );
}

inline void EvaluateRGBChannel3f( Vec3 *out, Vec3 p, Vec2 uv, const __global RGBChannel* color )
{
	*out = color->rgb;
}

inline void EvaluateRGBChannel1f( float *out, Vec3 p, Vec2 uv, const __global RGBChannel* color )
{
	*out = color->rgb.x;
}

#endif

