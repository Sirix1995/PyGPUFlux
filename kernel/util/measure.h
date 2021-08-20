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

/*
A detector is a light energy quantity measured during simulation.
A detector has no one-to-one relation to a photon. Instead, multiple photons may contribute to a single detector.
When running the simulation in parallel, for example on a GPU, a detector might be updated atomicaly by multiple threads.
To reduce coliding atomics, detector may have consist of multiple measurements. A thread may contribute to any of these measurements.
Note that for this to have any effect, measurements need to lie on different cache lines. 
To achive this, measurements with consequitive indices are shuffled throughout memory by mirroring their relevant index bits.
This results in maximum spacing between measurements belonging to one detector, resulting in minimal atomic collisions.
The the best memory/performance balance, detectors that are more likely to be updated should have more measurements.
*/

#ifndef _MEASURE_H
#define _MEASURE_H

#include "color/cie.h"
#include "color/spectralcolor.h"
#include "color/spectrum.h"

typedef struct
{
	int offset;
	int count;
}Detector;

#ifndef NUM_SENSITIVITYSPDS
	#define NUM_SENSITIVITYSPDS 1
#endif

#ifdef SPECTRAL

	#ifdef MEASURE_FULL_SPECTRUM
		
		typedef struct
		{
			//int cs;
			float dat[MEASURE_SPECTRUM_BINS];
		}Measurement;
		
	#else
	
		typedef struct
		{
			//int cs;
			float dat[NUM_SENSITIVITYSPDS];
		}Measurement;
		
	#endif
	
#else

	typedef struct
	{
		//int cs;
		Vec3 rgb;
	}Measurement;

#endif

typedef struct
{
	// sensitivity data
	SpectralDistribution curve[NUM_SENSITIVITYSPDS];
}MeasurementSensitivityCurve;

// shuffle the measurement locations
inline unsigned int ShuffleMeasurement( unsigned int value , int bits)
{
	// mirror the lower binary bits in value 
	value = ((value & 0xAAAAAAAA) >>  1) | ((value & 0x55555555) <<  1);
	value = ((value & 0xCCCCCCCC) >>  2) | ((value & 0x33333333) <<  2);
	value = ((value & 0xF0F0F0F0) >>  4) | ((value & 0x0F0F0F0F) <<  4);
	value = ((value & 0xFF00FF00) >>  8) | ((value & 0x00FF00FF) <<  8);
	value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
	return value >> (32-bits);
}

// get a measurement corresponding to the given detector
inline unsigned int GetMeasurementIdx( const __global Detector * detectors, unsigned int depth, int bits, unsigned int detectorIdx  )
{
	// get detector
	const __global Detector* detector = &detectors[detectorIdx];
	// select one measurement slot using round robbin
	return ShuffleMeasurement( detector->offset + get_global_id(0) % detector->count, bits ) + depth * (1 << (bits-1));
}


#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

/*
 * Deadlock free SIMT critical section using spinlock
 *	 The critial section is part of the loop
 *	 The critical section is executed as soon as it is aquired, preventing starvation due to SIMT execution
 */
 /*
inline void AtomicAddSpectrum( __global Measurement *measurement, const Spectrum *value, const Spectrum *spectrum, const __global MeasurementSensitivityCurve *sensitivitycurves )
{

#ifdef SPECTRAL

	#ifdef MEASURE_FULL_SPECTRUM
	
		// compute spectral buckets
		iSpectrum intervals = SampleSpectralIntervals( spectrum, MEASURE_MIN_LAMBDA, MEASURE_MAX_LAMBDA, MEASURE_SPECTRUM_BINS);
			
	#else
	
		// evaluate the contributions of each wavelength to each sensitivity curve
		float contributions[NUM_SENSITIVITYSPDS];
		for( int i = 0 ; i < NUM_SENSITIVITYSPDS ; i++ )
		{
			Spectrum sensitivity;
			EvalSpectrum( &sensitivity, spectrum, &(sensitivitycurves->curve[i]) );
			contributions[i] = specdot( &sensitivity, value );
		}
		
	#endif	
	
#else

	// compute rgb color
	Vec3 rgb;
	Spectral2RGB( &rgb, value, spectrum );
	
#endif
		
	// get critical section flag
	__global int *cs = &measurement->cs;
	
	int ret_val;
		
	// repeat
	do
	{
		// try to aquire critical section
		ret_val = atom_xchg( cs, 1 );
		
		// if critial section was free
		if( ret_val == 0 )
		{
		
	#ifdef SPECTRAL
	
		#ifdef MEASURE_FULL_SPECTRUM
			
			__global float* dat = (__global float*)(&measurement->dat);
			
			// contribute to spectrum
			ContributeSpectrum( dat, value, &intervals );
			
		#else
		
			__global float* dat = (__global float*)(&measurement->dat);
		
			// contribute to integrated spectra
			for( int i = 0 ; i < NUM_SENSITIVITYSPDS ; i++ )
				dat[i] += contributions[i];
		
		#endif
		
	#else
			
			__global float* out_color = (__global float*)(&measurement->rgb);
			
			// contribute to color
			out_color[0] += rgb.x;
			out_color[1] += rgb.y;
			out_color[2] += rgb.z;
			
	#endif
		
			// force writes
			write_mem_fence( CLK_GLOBAL_MEM_FENCE );
			
			// free critial section
			*cs = 0;
		}

		// force local writes and reads
		mem_fence( CLK_LOCAL_MEM_FENCE );
		
		// continue if critical section was in use
	}while( ret_val == 1 );
};
*/

inline void AtomicAddSpectrum( __global Measurement *measurement, const Spectrum *value, const Spectrum *spectrum, const __global MeasurementSensitivityCurve *sensitivitycurves )
{

#ifdef SPECTRAL

	#ifdef MEASURE_FULL_SPECTRUM
	
		// compute spectral buckets
		iSpectrum intervals = SampleSpectralIntervals( spectrum, MEASURE_MIN_LAMBDA, MEASURE_MAX_LAMBDA, MEASURE_SPECTRUM_BINS);
			
	#else
	
		// evaluate the contributions of each wavelength to each sensitivity curve
		float contributions[NUM_SENSITIVITYSPDS];
		for( int i = 0 ; i < NUM_SENSITIVITYSPDS ; i++ )
		{
			Spectrum sensitivity;
			EvalSpectrum( &sensitivity, spectrum, &(sensitivitycurves->curve[i]) );
			contributions[i] = specdot( &sensitivity, value );
		}
		
	#endif	
	
#else

	// compute rgb color
	Vec3 rgb;
	Spectral2RGB( &rgb, value, spectrum );
	
#endif
		
	#ifdef SPECTRAL
	
		#ifdef MEASURE_FULL_SPECTRUM
			
			__global float* dat = (__global float*)(&measurement->dat);
			
			// contribute to spectrum
			ContributeSpectrum( dat, value, &intervals );
			
		#else
		
			__global float* dat = (__global float*)(&measurement->dat);
		
			// contribute to integrated spectra
			for( int i = 0 ; i < NUM_SENSITIVITYSPDS ; i++ )
				AtomicAdd( &dat[i], contributions[i] );
		
		#endif
		
	#else
			
			__global float* out_color = (__global float*)(&measurement->rgb);

			// contribute to color			
			AtomicAdd( &out_color[0], rgb.x );
			AtomicAdd( &out_color[1], rgb.y );
			AtomicAdd( &out_color[2], rgb.z );
			
	#endif
};


#endif
