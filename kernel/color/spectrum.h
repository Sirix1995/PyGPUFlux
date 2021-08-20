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

#ifndef _SPECTRUM_H
#define _SPECTRUM_H

#include "light/light.h"
#include "util/util.h"
#include "math/vec1.h"
#include "math/vec3.h"
#include "math/vec4.h"
#include "util/sync.h"

#define SPECTRAL_WAVELENGTH_DEFAULT ((SPECTRAL_WAVELENGTH_MAX + SPECTRAL_WAVELENGTH_MIN) / 2)
#define SPECTRAL_WAVELENGTH_RANGE (SPECTRAL_WAVELENGTH_MAX - SPECTRAL_WAVELENGTH_MIN)
#define SPECTRAL_WAVELENGTH_STEP (SPECTRAL_WAVELENGTH_BINS / SPECTRAL_WAVELENGTH_RANGE)

typedef struct {
	Vec3 rgb;
	float power[SPECTRAL_WAVELENGTH_BINS];
} SpectralDistribution;

#ifdef SPECTRAL

	inline float SampleSpectrumf1( float *out, const float *spectrum, const __global SpectralDistribution *sp )
	{
		int bin = (int)((*spectrum - SPECTRAL_WAVELENGTH_MIN) / SPECTRAL_WAVELENGTH_STEP);
		
		bin = max( (int)(0) , bin );
		bin = min( bin, (int)(SPECTRAL_WAVELENGTH_BINS-1) );
		
		*out = sp->power[bin];
		return *out;
	}

	inline float4 SampleSpectrumf4( float4 *out, const float4 *spectrum, const __global SpectralDistribution *sp )
	{
		int4 bin = convert_int4((*spectrum - SPECTRAL_WAVELENGTH_MIN) / SPECTRAL_WAVELENGTH_STEP);
		
		bin = max( bin, (int4)(0) );
		bin = min( bin, (int4)(SPECTRAL_WAVELENGTH_BINS-1) );
		
		*out = 
			(float4)(
				sp->power[bin.x],
				sp->power[bin.y],
				sp->power[bin.z],
				sp->power[bin.w] );
		return *out;
	}
	
#else

	inline float SampleSpectrumf1( float *out, const float *spectrum, const __global SpectralDistribution *sp )
	{
		*out = sp->rgb.x;
		return *out;
	}

	inline float4 SampleSpectrumf4( float4 *out, const float4 *spectrum, const __global SpectralDistribution *sp )
	{
		(*out).x = sp->rgb.x;
		(*out).y = sp->rgb.y;
		(*out).z = sp->rgb.z;
		(*out).w = 1.f;
		return *out;
	}

#endif

int SampleSpectralIntervals1(const float *lSample, const float minLambda, const float maxLambda, const int bins)
{
	int id = (int)((((*lSample) - minLambda) / (maxLambda - minLambda)) * bins);
	id = min( id, (int)(bins-1));
	id = max( id, (int)(0));
	return id;
}

int4 SampleSpectralIntervals4(const float4 *lSample, const float minLambda, const float maxLambda, const int bins)
{
	int4 id = convert_int4((((*lSample) - minLambda) / (maxLambda - minLambda)) * bins);
	id = min( id, (int4)(bins-1,bins-1,bins-1,bins-1));
	id = max( id, (int4)(0,0,0,0));
				
	return id;
}

inline void ContributeSpectrum1( __global float *out, const float* in, const int* offset)
{
	AtomicAdd( &out[(*offset)], *in );
	//out[*offset] = *in;
}

inline void ContributeSpectrum4( __global float *out, const float4* in, const int4* offset)
{
	AtomicAdd( &out[(*offset).x], (*in).x );
	AtomicAdd( &out[(*offset).y], (*in).y );
	AtomicAdd( &out[(*offset).z], (*in).z );
	AtomicAdd( &out[(*offset).w], (*in).w );
	
	//out[(*offset).x] = (*in).x;
	//out[(*offset).y] = (*in).y;
	//out[(*offset).z] = (*in).z;
	//out[(*offset).w] = (*in).w;
}



inline float* MapUnitToSpectrumf1( float *out, float *in )
{
	*out = SPECTRAL_WAVELENGTH_MIN + (SPECTRAL_WAVELENGTH_MAX - SPECTRAL_WAVELENGTH_MIN) * (*in);
	return out;
}

inline float4* MapUnitToSpectrumf4( float4 *out, float4 *in )
{
	*out = SPECTRAL_WAVELENGTH_MIN + (SPECTRAL_WAVELENGTH_MAX - SPECTRAL_WAVELENGTH_MIN) * (*in); 
	return out;
}

inline float* SampleUnitSpectumf1( float *out, Random *rnd )
{
	*out = random1f( rnd );
	return out;
}

inline float4* SampleUnitStratifiedSpectumf4( float4 *out, Random *rnd )
{
	*out = ((float4)( 0, 1, 2, 3 ) + random4f( rnd ) ) / 4.f;
	return out;
}

#endif

