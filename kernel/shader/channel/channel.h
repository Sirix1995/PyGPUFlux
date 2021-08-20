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

#ifndef _CHANNEL_H
#define _CHANNEL_H

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

#define CHANNEL_TREE 0x10000000 // 1 0000 0000 0000 0000 0000 0000 0000

typedef int ChannelHandle;

typedef struct {
	int type;
	Vec3 m0, m1;
	ChannelHandle nextChannel;
} Channel;

inline bool isChannel(ChannelHandle channel)
{ return channel != -1; }

inline void evaluateUVTransforms( const __global Channel* channel, Vec2 *uv )
{
	// transform uv coordinates
	v2init( uv, 
		channel->m0.x * uv->x + channel->m0.y * uv->y + channel->m0.z,
		channel->m1.x * uv->x + channel->m1.y * uv->y + channel->m1.z );
}

#endif

