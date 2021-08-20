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

#ifndef _EVAL_CHANNEL_TREE_H
#define _EVAL_CHANNEL_TREE_H

#define CHANNEL_STACK_SIZE 15

typedef union
{
	int i;
	float f;
}StackItem; 

typedef union
{
	StackItem stack[CHANNEL_STACK_SIZE];
	int head;
}Stack;

/* Auxiliary functions for manipulating the stack */

inline void initStack( Stack *stack )
{
	stack->head = 0;
}

inline void pushInt( Stack *stack, int i )
{
	stack->stack[stack->head++].i = i;
}

inline void pushFloat( Stack *stack, float f )
{
	stack->stack[stack->head++].f = f;
}

inline int popInt( Stack *stack )
{
	return stack->stack[--stack->head].i;
}

inline float popFloat( Stack *stack )
{
	return stack->stack[--stack->head].f;
}

typedef struct 
{
	Channel base;
	float frequency;
	float phase;
} VolumeChannel;

typedef struct
{
	VolumeChannel base;
	Vec3 direction;
} GradientChannel;

typedef struct 
{
	Channel base;
} TurbulenceChannel;

typedef struct 
{
	Channel base;
	Vec3 m0, m1;
} UVChannel;

inline Vec3 turbulence( const __global TurbulenceChannel* turbulence, Vec3 in )
{
	return in;
}

/*
	Evaluate an rgba channel tree within a given environment

	in: environment
	in: channel tree root
	out: channel color
*/
inline void EvaluateChannelTree( float4 *out_color, const Environment *env, ChannelHandle channelHandle, const __global char *channels )
{
	// local environment, modified by channels
	
	// local space intersection point
	Vec3 lp = env->lp;
	// uv coordinates
	Vec2 uv = env->uv;
	// output color
	float4 color = 0;
		
	// channel stack
	Stack stack;
	initStack( &stack );

	// traverse the channel tree in postorder
	while( isChannel(channelHandle) )
	{
		// get channel
		const __global Channel* channel = (const __global Channel*)&(channels[channelHandle]);
		
		// traverse to the next channel node
		channelHandle = channel->nextChannel;
		
		// if this is a leaf, reset the environment
		if( channel->type & CHANNEL_LEAF )
		{
			lp = env->lp;
			uv = env->uv;
			color = 0;
		}
				
		float4 result = 0;
				
		// evaluate channel
		if( channel->type & CHANNEL_VOLUME_FUNCTION )
		{
			// get volume channel
			const __global VolumeChannel* volumeChannel = (const __global VolumeChannel*)channel;
			
			// evaluate volume channel
			float v;
			switch( channel->type & CHANNEL_TYPE )
			{
				case CHANNEL_GRADIENT:
				{
					const __global GradientChannel* gradientChannel = (const __global GradientChannel*)volumeChannel;
					Vec3 direction = gradientChannel->direction;
					v = v3dot( &direction, &lp );
				}
				// TODO: add different volume functions
				default:
					v = 0.f;
			};
			v = v * volumeChannel->frequency + volumeChannel->phase;
			
			// TODO: apply wave function
			
			result = v;
		}
		else if( channel->type & CHANNEL_TURBULENCE )
		{
			// get turbulence channel
			const __global TurbulenceChannel* turbulenceChannel = (const __global TurbulenceChannel*)channel;
			
			Vec3 in;
			
			// get input
			if( channel->type & MODIFY_UV )
				v3init( &in, uv.x, uv.y, 0.f );
			else if( channel->type & MODIFY_XYZ )
				in = lp;
				
			// perform turbulence
			in = turbulence( turbulenceChannel, in );
			result = (float4)( in.x, in.y, in.z, 1.f );
			
		}
		else if( channel->type & CHANNEL_UV )
		{
			// get uv channel
			const __global UVChannel* uvChannel = (const __global UVChannel*)channel;
			
			// transform uv coordinates
			Vec2 out;
			v2init( &out, 
				uvChannel->m0.x * uv.x + uvChannel->m0.y * uv.y + uvChannel->m0.z,
				uvChannel->m1.x * uv.x + uvChannel->m1.y * uv.y + uvChannel->m1.z );
				
			result = (float4)( out.x, out.y, 1.f, 1.f );
		}
		else if( channel->type & CHANNEL_RGB )
		{
			// get rgb channel
			const __global RGBChannel* rgbChannel = (const __global RGBChannel*)channel;
			
			result = (float4)( rgbChannel->rgb.x, rgbChannel->rgb.y, rgbChannel->rgb.z, 1.f );
		}
		else if( channel->type & CHANNEL_IMAGE_MAP )
		{
			// get image channel
			const __global ImageChannel* imageChannel = (const __global ImageChannel*)channel;
			
			EvaluateImage( &result, uv, &imageChannel->image );
		}
		else if( channel->type & CHANNEL_HDRIMAGE_MAP )
		{
			// get image channel
			const __global HDRImageChannel* hdrImageChannel = (const __global HDRImageChannel*)channel;
			
			Vec3 out;
			EvaluateHDRImage( &out, uv, &hdrImageChannel->image );
			result = (float4)(out.x, out.y, out.z, 1.f);
		}
		else if( channel->type & CHANNEL_BLEND_BRANCH )
		{
			// use x to search for the two blend channels and push them on the stack
			const __global BlendChannel* blendChannel = (const __global BlendChannel*)channel;
		
			// get the blending parameter
			const float t = lp.x;
			
			float blend = 0;
			ChannelHandle first = -1;
			ChannelHandle second = -1;
			
			// find first larger item
			int item = 0;
			for( ; item < blendChannel->n && blendChannel->items[item].threshold < t; item++ );
			
			// select the blend channels
			if( item == 0 )
			{
				first = blendChannel->items[0].channel;
				blend = 0;
			}
			else if( item == blendChannel->n )
			{
				first = blendChannel->items[blendChannel->n-1].channel;
				blend = 0;
			}
			else
			{
				first = blendChannel->items[item-1].channel;
				second = blendChannel->items[item].channel;
				
				const float a = blendChannel->items[item-1].threshold;
				const float b = blendChannel->items[item].threshold;
				
				blend = (t - a) / (b - a);
			}
			
			// push blend factor
			pushFloat( &stack, blend );
			
			// push second blend item
			pushInt( &stack, second );
			
			// start first blend item
			channelHandle = first;
		}
		else if( channel->type & CHANNEL_BLEND_ITEM )
		{
			// pop second blend item
			channelHandle = popInt( &stack );
		
			// push color in reverse order
			pushFloat( &stack, color.w );
			pushFloat( &stack, color.z );
			pushFloat( &stack, color.y );
			pushFloat( &stack, color.x );
		}
		else if( channel->type & CHANNEL_BLEND_MERGE )
		{
			// pop the first blend item from the stack
			float4 color_first = (float4)( 
				popFloat( &stack ),
				popFloat( &stack ),
				popFloat( &stack ),
				popFloat( &stack ) );
				
			// pop the blend factor from the stack
			float blend = popFloat( &stack );
			
			// blend the items together
			result = color * blend + color_first * (1 - blend);
		}
		
		
		// modify local environment
		if( channel->type & MODIFY_XYZ )
			v3init( &lp, result.x, result.y, result.z );
		else if( channel->type & MODIFY_UV )
			v2init( &uv, result.x, result.y );
		else if( channel->type & MODIFY_RGBA )
			color = result;
	};
		
	// return the output color
	*out_color = color;
}

#endif

/*
#define CHANNEL_SPECTRAL 0x100  // 0000 0000 0000 0000 0001 0000 0000

#define CHANNEL_IMAGE_MAP 0		// 0000 0000 0000 0000 0000 0000 0000
#define CHANNEL_RGB 1			// 0000 0000 0000 0000 0000 0000 0001
#define CHANNEL_UV 2			// 0000 0000 0000 0000 0000 0000 0010
#define CHANNEL_HDRIMAGE_MAP 3  // 0000 0000 0000 0000 0000 0000 0011
#define CHANNEL_BLEND 4			// 0000 0000 0000 0000 0000 0000 0100
#define CHANNEL_TURBULENCE		5 // 0000 0000 0000 0000 0000 0000 0101
#define CHANNEL_BLEND_BRANCH	6 // 0000 0000 0000 0000 0000 0000 0110
#define CHANNEL_BLEND_ITEM		7 // 0000 0000 0000 0000 0000 0000 0111
#define CHANNEL_BLEND_MERGE		8 // 0000 0000 0000 0000 0000 0000 1000

#define CHANNEL_VOLUME_FUNCTION 0x80 						  // 0000 0000 0000 0000 0000 1000 0000
#define CHANNEL_GRADIENT		(1 | CHANNEL_VOLUME_FUNCTION) // 0000 0000 0000 0000 0000 1000 0001

#define CHANNEL_TYPE 0xFF		// 0000 0000 0000 0000 0000 1111 1111

#define MODIFY_XYZ	 0x1000000 // 0001 0000 0000 0000 0000 0000 0000
#define MODIFY_UV	 0x2000000 // 0010 0000 0000 0000 0000 0000 0000
#define MODIFY_RGBA	 0x4000000 // 0100 0000 0000 0000 0000 0000 0000
#define CHANNEL_LEAF 0x8000000 // 1000 0000 0000 0000 0000 0000 0000
*/