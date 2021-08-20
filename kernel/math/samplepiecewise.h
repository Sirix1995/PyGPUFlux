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

#ifndef _SAMPLE_MAP_H
#define _SAMPLE_MAP_H

#include "util/util.h"

// get sample probability within an interval
inline float PiecewiseDistributionIntervalSampleProbability( const __global float* cdf, int length , int idx )
{
	float low = idx?cdf[idx-1]:0.f;
	float high = cdf[idx];
	
	return (high-low) * invSafe(cdf[length-1]);
}

// sample a random interval on a regular piecewise constant distribution
inline int SamplePiecewiseDistributionInterval( const __global float* cdf, int length , float r )
{
	int low = 0, high = length;
	r *= cdf[length-1];
	
	while( low + 1 < high )
	{
		int middle = (low + high) >> 1;
		float cumtotal = cdf[middle - 1];
		if( cumtotal >= r )
			high = middle;
		else
			low = middle;
	};
	
	return low;
};

// sample a random point on a regular piecewise constant distribution
inline float SamplePiecewiseDistribution1( float *out, const __global float* cdf, int length , float r )
{
	int idx = SamplePiecewiseDistributionInterval( cdf, length, r );
	
	r *= cdf[length-1];
	
	float low_p = idx==0?0:cdf[idx - 1];
	float high_p = cdf[idx];
	
	*out = (idx + (r - low_p) * invSafe(high_p-low_p)) / length;
		
	return length * (high_p-low_p);
}

// sample a random point on a regular piecewise constant distribution
inline float4 SamplePiecewiseDistribution4( float4 *out, const __global float* cdf, int length , float4 r )
{
	float x,y,z,w;
	float4 p;
	p.x = SamplePiecewiseDistribution1(&x, cdf, length, r.x );
	p.y = SamplePiecewiseDistribution1(&y, cdf, length, r.y );
	p.z = SamplePiecewiseDistribution1(&z, cdf, length, r.z );
	p.w = SamplePiecewiseDistribution1(&w, cdf, length, r.w );
	*out = (float4)(x,y,z,w);
	return p;
}

#endif
