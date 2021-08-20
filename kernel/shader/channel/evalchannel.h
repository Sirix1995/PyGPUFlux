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

#ifndef _EVAL_CHANNEL_H
#define _EVAL_CHANNEL_H

#include "shader/channel/channel.h"
#include "math/vec3.h"
#include "math/vec2.h"
#include "color/color.h"

#include "shader/channel/rgbchannel.h"
#include "shader/channel/imagechannel.h"
#include "shader/channel/hdrimagechannel.h"

void EvaluateChannel4f( float4 *out, Vec3 p, Vec2 uv, ChannelHandle channelHandle, const __global char *channels )
{
	if( !isChannel(channelHandle) )
	{
		*out = (float4)(0.f,0.f,0.f,0.f);
		return;
	}
	
	__global Channel* channel = (__global Channel*)&(channels[channelHandle]);
	evaluateUVTransforms( channel, &uv);

	// switch on channel type
	switch( channel->type & CHANNEL_TYPE )
	{
		case CHANNEL_IMAGE_MAP:
			EvaluateImageChannel4f( out, p, uv, (const __global ImageChannel*)channel );
			break;
		case CHANNEL_RGB:
			EvaluateRGBChannel4f( out, p, uv, (const __global RGBChannel*)channel );
			break;
		case CHANNEL_HDRIMAGE_MAP:
			EvaluateHDRImageChannel4f( out, p, uv, (const __global HDRImageChannel*)channel );
			break;
	};
}

void EvaluateChannel3f( Vec3 *out, Vec3 p, Vec2 uv, ChannelHandle channelHandle, const __global char *channels )
{
	if( !isChannel(channelHandle) )
	{
		v3init(out, 0.f, 0.f, 0.f);
		return;
	}
	
	__global Channel* channel = (__global Channel*)&(channels[channelHandle]);
	evaluateUVTransforms( channel, &uv);
	
	// switch on channel type
	switch( channel->type & CHANNEL_TYPE )
	{
		case CHANNEL_IMAGE_MAP:
			EvaluateImageChannel3f( out, p, uv, (const __global ImageChannel*)channel );
			break;
		case CHANNEL_RGB:
			EvaluateRGBChannel3f( out, p, uv, (const __global RGBChannel*)channel );
			break;
		case CHANNEL_HDRIMAGE_MAP:
			EvaluateHDRImageChannel3f( out, p, uv, (const __global HDRImageChannel*)channel );
			break;
	};
}

void EvaluateChannel1f( float *out, Vec3 p, Vec2 uv, ChannelHandle channelHandle, const __global char *channels )
{
	if( !isChannel(channelHandle) )
	{
		*out = 0.f;
		return;
	}
	
	__global Channel* channel = (__global Channel*)&(channels[channelHandle]);
	evaluateUVTransforms( channel, &uv);

	// switch on channel type
	switch( channel->type & CHANNEL_TYPE )
	{
		case CHANNEL_IMAGE_MAP:
			EvaluateImageChannel1f( out, p, uv, (const __global ImageChannel*)channel );
			break;
		case CHANNEL_RGB:
			EvaluateRGBChannel1f( out, p, uv, (const __global RGBChannel*)channel );
			break;
		case CHANNEL_HDRIMAGE_MAP:
			EvaluateHDRImageChannel1f( out, p, uv, (const __global HDRImageChannel*)channel );
			break;
	};
}

#endif
