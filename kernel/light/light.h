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

#ifndef _LIGHT_H
#define _LIGHT_H

#define LIGHT_POINT 0
#define LIGHT_DIRECTIONAL 1
#define LIGHT_SPOT 2
#define LIGHT_AREA 3
#define LIGHT_PHYSICAL 4
#define LIGHT_SKY 5
#define LIGHT_SPECTRAL 6

#include "math/samplepiecewise.h"

typedef struct {
	// light type
	int type;
	// number of samples per light
	int samples;
	// world to object transformation matrix
	Mat34 mo2w, mw2o;
	//  power distribution, integrates to total light source power (per unit area for infinite light sources)
	Vec3 power;
	// discretized cumulative spectral importance
	float spectral_cdf[SPECTRAL_WAVELENGTH_BINS];
} Light;

typedef struct {
	// The location of the light in global coordinates
	Vec3 origin;
	
	// The color of the light
	Vec3 color;
} LightSample;

inline void TransformLightDirection2Object( Vec3* out, const __global Light *light, const Vec3* dir )
{
	// get world to local transformation
	Mat33 mw2o = light->mw2o.m33;
	
	// transform light direction to local space
	//m34vmul( out , &mw2o , dir );
	m33vmul( out , &mw2o , dir );
	v3norm( out, out );
}

inline void TransformLightDirection2World( Vec3* out, const __global Light *light, const Vec3* dir )
{
	// get local to world transformation
	Mat33 mo2w = light->mo2w.m33;
	
	// transform light direction to local space
	m33vmul( out , &mo2w , dir );
	v3norm( out, out );
}

// sample random light, proportional to power
__global Light* sampleLightSource( 
	float *prb, // out: probability of selecting the returned lightsource
	float r, // in: uniform random value in [0,1]
	int nl, // in: number of light sources
	const __global char *lights, // in: light source data
	const __global int *lightOffsets, // in: start offset for each light in light data
	const __global float *cumLightPower // in: cumulative power approximation buffer for IS
	)
{
	int idx = SamplePiecewiseDistributionInterval( cumLightPower, nl, r );
	*prb = PiecewiseDistributionIntervalSampleProbability( cumLightPower, nl, idx );
	return (__global Light*)(&lights[ lightOffsets[idx] ]);
}

#endif

