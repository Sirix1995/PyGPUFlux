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

#ifndef _SPECTRAL_CHANNEL_H
#define _SPECTRAL_CHANNEL_H

#include "math/vec3.h"
#include "shader/channel/channel.h"
#include "color/spectrum.h"

typedef struct {
	Channel base;
	// spectral distribution
	SpectralDistribution spectrum;
} SpectralChannel;

inline void EvaluateSpectralChannel( Spectrum *out_color, const Spectrum *spectrum, Vec3 p, Vec2 uv, const __global SpectralChannel* channel )
{
	EvalSpectrum( out_color, spectrum, &(channel->spectrum) );
}

inline void EvaluateSpectralChannel1f( float *out, const Spectrum *spectrum, Vec3 p, Vec2 uv, const __global SpectralChannel* channel )
{
#if defined(SPECTRUM_DISPERSION)
	float lambda = *spectrum;
#else
	float lambda = (*spectrum).x;
#endif
	
	// sample spectral material
	SampleSpectrumf1( out, &lambda, &channel->spectrum );
}


#endif

