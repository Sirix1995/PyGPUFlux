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

#ifndef _SENSOR_H
#define _SENSOR_H

#include "color/color.h"
#include "util/measure.h"

typedef struct
{
	int group_idx;
	// world to object transformation matrix
	Mat34 m;
	// sense on two sides	
	bool twoSided;
	// sensitivity for color channels
	Vec3 color;
	// sensitivity distibution parameter
	float exponent;
}Sensor;

inline bool computeSensorIntersect( float length, const Ray *r, const __global Sensor *sensor )
{ 
	float t = - v3dot( &r->o, &r->d ) / v3dot( &r->d, &r->d ); // intersect with oriented disk
	//float t = -r->o.z; // intersect with disk
	
	Vec3 p;
	rmarch( &p , t , r );

	if (v3dot( &p, &p ) <= 1)
	{
		if( 0 < t && t < length )
			return true; 
	}

	return false; 
}

// NOTE: computes normal in world space
inline void computeSensorNormal( Vec3 *norm, const __global Sensor *sensor )
{
	// set normal to (0,0,1), transformed by the transpose of matrix
	v3init( norm, sensor->m.m33.m20, sensor->m.m33.m21, sensor->m.m33.m22 );
	
	// normalize normal
	v3norm( norm , norm );
}

// computes sensor sensitivity
inline void computeSensorSensititivy( Spectrum *irad, const Spectrum *spectrum, const Vec3 *norm, const Vec3 *in, const __global Sensor *sensor )
{
	float cos = -v3dot( norm , in );
	if (cos <= 0)
	{
		cos = sensor->twoSided ? fabs (cos) : 0;
	}
	float f = pow (cos, sensor->exponent) * (sensor->exponent + 1) * F_M_1_2PI;
	if (sensor->twoSided)
	{
		f *= 0.5f;
	}
	
	Vec3 color = sensor->color;
	
	// convert rgba to spectral representation
	Spectrum sdf;
	RGB2Spectral( &sdf, &color , spectrum );
	
	specsmul( irad, &sdf, f );
}

 // return true iff shadow ray is blocked
void computeSensorIntersects( DEBUG_PAR ,
	__global Measurement* irradiance_buffer,
	const __global Detector *detectors,
	const __global MeasurementSensitivityCurve *sensitivityCurves,
	int depth,
	int measurementBits,
	Spectrum *irradiance, 
	const Spectrum *spectrum,
	int start , int num , 
	const __global Sensor *sensors,
	float length,
	const Ray *r
	)
{
	for( int sidx = start ; sidx < start + num ; sidx++ )
	{
		const __global Sensor *sensor = &sensors[sidx];
	
		Mat34 m = sensor->m;
		Ray lr;
		// transform ray to object space
		m34rmul( &lr , &m , r );	
		
		// compute intersection with sensor
		if( computeSensorIntersect( length, &lr, sensor ) )
		{
			// get sensor normal
			Vec3 norm;
			Spectrum irad;
			computeSensorNormal( &norm, sensor );
			
			// compute sensor irradiance sensitivity
			computeSensorSensititivy( &irad, spectrum, &norm, &lr.d, sensor );
			
			if( specsum( &irad ) > 0 )
			{
				// compute sensed irradiance
				specmul( &irad, &irad, irradiance );
	
				Vec3 rgbirad;
				Spectral2RGB( &rgbirad , &irad, spectrum );
	
				// accumulate sensed irradiance
				int measurementIdx = GetMeasurementIdx(detectors, depth, measurementBits, sensor->group_idx);
				AtomicAddSpectrum( &irradiance_buffer[measurementIdx], &irad, spectrum, sensitivityCurves );
				//AtomicAddVec3( &irradiance_buffer[measurementIdx], &rgbirad );
			}
		}
	}
}

#endif