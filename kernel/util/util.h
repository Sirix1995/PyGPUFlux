/*
 *  Copyright (c) 2011 Dietger van Antwerpen (dietger@xs4all.nl)
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 
#ifndef _UTIL_H
#define _UTIL_H

#include "util/rnd.h"

inline void swapInt( int *a, int *b )
{ int tmp = *a;*a = *b; *b = tmp; }

inline void swapFloat( float *a, float *b )
{ float tmp = *a;*a = *b; *b = tmp; }

inline float interpolate(float f00, float f01, float f10, float f11, float d, float e)
{
	return f00*(1-d)*(1-e) + f01*(1-d)*e + f10*d*(1-e) + f11*d*e;
}

inline float invSafe(float x)
{
	return fabs(x)<FLT_EPSILON?0:1.f/x;
}

#endif