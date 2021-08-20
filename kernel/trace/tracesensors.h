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

#ifndef _TRACE_SENSORS_H
#define _TRACE_SENSORS_H

#include "geo/sensor.h"
#include "util/util.h"
#include "color/color.h"

#define SPATIAL_SENSOR_STRUCTURE

#include "trace/bvh/tracesensors.h"

void traceSensor( DEBUG_PAR,
	__global Measurement * irradiance_buffer,
	const __global Detector *detectors,
	const __global MeasurementSensitivityCurve *sensitivityCurves,
	int depth,
	int measurementBits,
	Spectrum *irradiance,
	const Spectrum *spectrum,
	float length,
	const Ray *r,
	int ns ,
	const __global Sensor *sensors,
	const __global BVHNode *bvh,
	int root )
{

#ifdef SPATIAL_SENSOR_STRUCTURE
	bvhTraceSensor( DEBUG_ARG, irradiance_buffer, detectors, sensitivityCurves, depth, measurementBits, irradiance, spectrum, length, r, sensors, bvh, root );
#else
	computeSensorIntersects( DEBUG_ARG , irradiance_buffer, detectors, sensitivityCurves, depth, measurementBits, irradiance, spectrum, 0, ns, sensors, length, r );
#endif

}


#endif