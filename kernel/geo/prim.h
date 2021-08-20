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

#ifndef _PRIM_H
#define _PRIM_H

#define  PRIM_TRANSFORMABLE 0x80
#define  PRIM_NOP 0x100
#define  PRIM_NOP_2 0x200
#define  PRIM_NOP_MASK (~(PRIM_NOP|PRIM_NOP_2))

#define  PRIM_PLANE (PRIM_TRANSFORMABLE | 1)
#define  PRIM_SPHERE (PRIM_TRANSFORMABLE | 2)
#define  PRIM_FRUSTUM (PRIM_TRANSFORMABLE | 3)
#define  PRIM_BOX (PRIM_TRANSFORMABLE | 4)
#define  PRIM_TRIANGLE (5)
#define  PRIM_PARALLEL (6)

#include "math/ray.h"
#include "math/math.h"
#include "math/vec2.h"

#include "util/measure.h"
#include "geo/aabb.h"

typedef struct
{
	int type;
	int group_idx;
	int shader_offset;
	float IOR;
	AABB aabb;
	// world to object transformation matrix
	Mat34 m;
}Prim;

typedef struct
{
	float t;
	const __global Prim *prim;
	union
	{
		int side;
		Vec2 uv;
	};
}Intc;

inline Intc* intc_init( Intc *intc, float t, const __global Prim *prim )
{ intc->t = t; intc->prim = prim; return intc; }

#endif