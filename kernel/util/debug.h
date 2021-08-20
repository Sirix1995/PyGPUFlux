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

#ifndef _DEBUG_H
#define _DEBUG_H

#define DEBUG_PAR __global Debug *debug
#define DEBUG_ARG debug

typedef struct 
{
	union
	{
		int i;
		int2 i2;
		int4 i4;
		float f;
		float2 f2;
		float4 f4;
	};
}Debug;

inline int debug_index()
{ return get_global_id(0); }

#define DEBUG debug[debug_index()]
#define DEBUG_INT debug[debug_index()].i
#define DEBUG_INT2 debug[debug_index()].i2
#define DEBUG_INT4 debug[debug_index()].i4
#define DEBUG_FLOAT debug[debug_index()].f
#define DEBUG_FLOAT2 debug[debug_index()].f2
#define DEBUG_FLOAT4 debug[debug_index()].f4

#endif