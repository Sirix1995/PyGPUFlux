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
#include "color/spectrum.h"

#include "shader/shaderenv.h"

#include "shader/channel/blendchannel.h"
#include "shader/channel/rgbchannel.h"
#include "shader/channel/imagechannel.h"
#include "shader/channel/hdrimagechannel.h"
#include "shader/channel/spectralchannel.h"
#include "shader/channel/evalchanneltree.h"

void EvaluateAlphaColorChannelChain( float *out_alpha, Spectrum *out_color, const Spectrum *spectrum, const Environment *env, ChannelHandle channelHandle, const __global char *channels )
{
	Vec3 p = env->p;
	Vec2 uv = env->uv;

	*out_alpha = 1.f;

	if( !isChannel(channelHandle) )
	{
		speczero( out_color );
		return;
	}
	
	const __global Channel* channel = (const __global Channel*)&(channels[channelHandle]);
	evaluateUVTransforms( channel, &uv);

	if( channel->type & CHANNEL_SPECTRAL )
	{
		EvaluateSpectralChannel( out_color, spectrum, p, uv, (const __global SpectralChannel*)channel );
	}
	/*	
	else if( channel->type & CHANNEL_TREE )
	{
		// sample rgba material
		float4 rgba;
		EvaluateChannelTree( &rgba, env, channelHandle, channels );
		
		*out_alpha = rgba.w;
		
		Vec3 rgb;
		v3initf4( &rgb, &rgba );
		
		// convert rgba to spectral representation
		RGB2Spectral( out_color, &rgb , spectrum );
	}
	*/
	else
	{
		// sample rgba material
		float4 rgba;
		
		// switch on channel type
		switch( channel->type & CHANNEL_TYPE )
		{
			case CHANNEL_IMAGE_MAP:
				EvaluateImageChannel4f( &rgba, p, uv, (const __global ImageChannel*)channel );
				break;
			case CHANNEL_RGB:
				EvaluateRGBChannel4f( &rgba, p, uv, (const __global RGBChannel*)channel );
				break;
			case CHANNEL_HDRIMAGE_MAP:
				EvaluateHDRImageChannel4f( &rgba, p, uv, (const __global HDRImageChannel*)channel );
				break;
		};
		
		*out_alpha = rgba.w;
		
		Vec3 rgb;
		v3initf4( &rgb, &rgba );
		
		// convert rgba to spectral representation
		RGB2Spectral( out_color, &rgb , spectrum );
	}
}

void EvaluateAlphaColorChannel( float *out_alpha, Spectrum *out_color, const Spectrum *spectrum, const Environment *env, ChannelHandle channelHandle, const __global char *channels )
{
	*out_alpha = 1.f;

	if( !isChannel(channelHandle) )
	{
		speczero( out_color );
		return;
	}
	
	const __global Channel* channel = (const __global Channel*)&(channels[channelHandle]);
	
	const float t = env->lp.x;
	
	float blend = 0;
	ChannelHandle first = -1;
	ChannelHandle second = -1;
	
	// check if blend channel
	if( (channel->type & CHANNEL_TYPE) == CHANNEL_BLEND )
	{
		const __global BlendChannel* blendchannel = (const __global BlendChannel*)channel;
	
		// find first larger item
		int item = 0;
		for( ; item < blendchannel->n && blendchannel->items[item].threshold < t; item++ );
		
		// select the blend channels
		if( item == 0 )
		{
			first = blendchannel->items[0].channel;
			blend = 0;
		}
		else if( item == blendchannel->n )
		{
			first = blendchannel->items[blendchannel->n-1].channel;
			blend = 0;
		}
		else
		{
			first = blendchannel->items[item-1].channel;
			second = blendchannel->items[item].channel;
			
			const float a = blendchannel->items[item-1].threshold;
			const float b = blendchannel->items[item].threshold;
			
			blend = (t - a) / (b - a);
		}
	}
	else
	{
		first = channelHandle;
		blend = 0;
	}
	
	// evaluate the first channel
	float alpha;
	Spectrum color;
	EvaluateAlphaColorChannelChain( &alpha, &color, spectrum, env, first, channels );
	
	// accumulate the first channel
	*out_alpha = (1.f - blend) * alpha;
	*out_color = (1.f - blend) * color;
		
	// evaluate the second channel
	EvaluateAlphaColorChannelChain( &alpha, &color, spectrum, env, second, channels );
	
	// accumulate the second channel
	*out_alpha += (blend) * alpha;
	*out_color += (blend) * color;
}

inline void EvaluateColorChannel( Spectrum *out_color, const Spectrum *spectrum, const Environment *env, ChannelHandle channelHandle, const __global char *channels )
{
	float alpha;
	EvaluateAlphaColorChannel( &alpha , out_color, spectrum, env, channelHandle, channels );
}

void EvaluateChannel( float *out, const Spectrum *spectrum, const Environment *env, ChannelHandle channelHandle, const __global char *channels )
{
	Vec3 p = env->p;
	Vec2 uv = env->uv;
	
	if( !isChannel(channelHandle) )
	{
		*out = 0.f;
		return;
	}
	
	const __global Channel* channel = (const __global Channel*)&(channels[channelHandle]);
	evaluateUVTransforms( channel, &uv);

	if( channel->type & CHANNEL_SPECTRAL )
	{
		EvaluateSpectralChannel1f( out, spectrum, p, uv, (__global SpectralChannel*)channel );
	}
	else
	{
		// switch on channel type
		switch( channel->type & CHANNEL_TYPE )
		{
			case CHANNEL_IMAGE_MAP:
				EvaluateImageChannel1f( out, p, uv, (__global ImageChannel*)channel );
				break;
			case CHANNEL_RGB:
				EvaluateRGBChannel1f( out, p, uv, (__global RGBChannel*)channel );
				break;
			case CHANNEL_HDRIMAGE_MAP:
				EvaluateHDRImageChannel1f( out, p, uv, (const __global HDRImageChannel*)channel );
				break;
		};
	}
}

#endif
