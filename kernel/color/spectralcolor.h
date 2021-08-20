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

#ifndef _SPECTRAL_COLOR_H
#define _SPECTRAL_COLOR_H

#include "color/spectrum.h"
#include "color/smits.h"


#if defined(SPECTRUM_DISPERSION)
	#define SPECTAL_CHANNELS 1
	
	#define Spectrum float
	#define iSpectrum int
	#define speczero(v) v1zero(v)
	#define specone(v) v1one(v)
	#define specneg(v,a) v1neg(v,a)
	#define specadd(v,a,b) v1add(v,a,b)
	#define specsub(v,a,b) v1sub(v,a,b)
	#define specmul(v,a,b) v1mul(v,a,b)
	#define specdiv(v,a,b) v1div(v,a,b)
	#define specsmul(v,a,s) v1smul(v,a,s)
	#define specmad(v,a,b,c) v1mad(v,a,b,c)
	#define specsmad(v,a,s,b) v1smad(v,a,s,b)
	#define specdot(a,b) v1dot(a,b)
	#define specsum(a) v1sum(a)
	#define specclamp(v,a) v1clamp(v,a)
	#define specset(v,a) v1init(v,a)
	#define RGB2Spectral(out,in,spec) RGBtoSpectralf1(out,in,spec)
	#define Spectral2RGB(out,in,spec) Spectralf1toRGB(out,in,spec)
	#define EvalSpectrum(out,in,spectrum) SampleSpectrumf1(out,in,spectrum)
	#define ContributeSpectrum(out,in,offset) ContributeSpectrum1(out,in,offset)
	#define SampleSpectralIntervals(in,minlambda,maxlambda,bins) SampleSpectralIntervals1(in,minlambda,maxlambda,bins)
	#define SampleUnitSpectum(out,in) SampleUnitSpectumf1(out,in)
	#define MapUnit2Spectrum(out,in) MapUnitToSpectrumf1(out,in)
	#define	ImportanceSampleSpectrum(out,cdf,n,in) SamplePiecewiseDistribution1(out,cdf,n,in)
#else
	#ifdef SPECTRAL
		#define SPECTAL_CHANNELS 4
	#else
		#define SPECTAL_CHANNELS 3
	#endif
	
	#define Spectrum float4
	#define iSpectrum int4
	#define speczero(v) v4zero(v)
	#define specone(v) v4one(v)
	#define specneg(v,a) v4neg(v,a)
	#define specadd(v,a,b) v4add(v,a,b)
	#define specsub(v,a,b) v4sub(v,a,b)
	#define specmul(v,a,b) v4mul(v,a,b)
	#define specdiv(v,a,b) v4div(v,a,b)
	#define specsmul(v,a,s) v4smul(v,a,s)
	#define specmad(v,a,b,c) v4mad(v,a,b,c)
	#define specsmad(v,a,s,b) v4smad(v,a,s,b)
	#define specdot(a,b) v4dot(a,b)
	#define specsum(a) v4sum(a)
	#define specclamp(v,a) v4clamp(v,a)
	#define specset(v,a) v1init(v,a,a,a,a)
	#define RGB2Spectral(out,in,spec) RGBtoSpectralf4(out,in,spec)
	#define Spectral2RGB(out,in,spec) Spectralf4toRGB(out,in,spec)
	#define EvalSpectrum(out,in,spectrum) SampleSpectrumf4(out,in,spectrum)
	#define ContributeSpectrum(out,in,offset) ContributeSpectrum4(out,in,offset)
	#define SampleSpectralIntervals(in,minlambda,maxlambda,bins) SampleSpectralIntervals4(in,minlambda,maxlambda,bins)
	#define SampleUnitSpectum(out,in) SampleUnitStratifiedSpectumf4(out,in)
	#define MapUnit2Spectrum(out,in) MapUnitToSpectrumf4(out,in)
	#define	ImportanceSampleSpectrum(out,cdf,n,in) SamplePiecewiseDistribution4(out,cdf,n,in)
#endif

inline Spectrum* SampleSpectrum(Spectrum *out, Random *rnd )
{
	SampleUnitSpectum( out, rnd );
	MapUnit2Spectrum( out, out );
	return out;
}

#endif