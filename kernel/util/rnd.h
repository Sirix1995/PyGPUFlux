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
 
#ifndef _RND_H
#define _RND_H

// Random generator from 'The KISS generator'(1993) by G. Marsaglia and A. Zaman
typedef struct 
{
	uint4 data;
}Random;

inline void initRandom( Random *rnd, int idx , int aSeed )
{
	rnd->data.x = 12872141u + aSeed * 426997u + aSeed;
	rnd->data.y = 2611909u + aSeed * 14910827u + idx * 1887143u + aSeed;
	rnd->data.z = 1010567u + aSeed * 2757577u + idx * 45751u + aSeed;
	rnd->data.w = 416191069u;
}

inline unsigned int random1i( Random *rnd )
{
	rnd->data.z = (36969 * (rnd->data.z & 65535) + (rnd->data.z >> 16)) << 16;
	rnd->data.w = 18000 * (rnd->data.w & 65535) + (rnd->data.w >> 16) & 65535;
	rnd->data.x = 69069 * rnd->data.x + 1234567;
	rnd->data.y = (rnd->data.y = (rnd->data.y = rnd->data.y ^ (rnd->data.y << 17)) ^ (rnd->data.y >> 13)) ^ (rnd->data.y << 5);
	return ((rnd->data.z + rnd->data.w) ^ rnd->data.x + rnd->data.y);
}

inline float random1f( Random *rnd )
{
	return random1i(rnd) * 2.328306E-10f;
}

inline float2 random2f( Random *rnd )
{
	float a = random1f( rnd );
	float b = random1f( rnd );
	
	return (float2)(a,b);
}

float4 random4f( Random *rnd )
{
	float2 a = random2f( rnd );
	float2 b = random2f( rnd );
	return (float4)( a.x, a.y, b.x, b.y );
}

#endif