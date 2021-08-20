#define GPU_KERNEL

#ifdef SPECTRUM_DISPERSION
	#define REFRACTION
#endif

#include "util/debug.h"
#include "util/sync.h"

#include "common/settings.h"

#include "geo/intersect.h"
#include "geo/normaluv.h"

#include "trace/trace.h"
#include "trace/tracesensors.h"

#include "shader/shader.h"
#include "shader/samplebsdf.h"
#include "shader/samplemaxbsdf.h"

#include "light/genlight.h"

#include "common/intersectenv.h"

__kernel void compute( DEBUG_PAR ,
	int nthreads,
	int sampleOffset,
	int nsamples,
	// output buffers
	__global Measurement *power,
	__global Measurement *irradiance,
	// detectors
	__global Detector *detectors,
	int measurementBits,
	// scene
	int np , int ninfp ,
	__global char *prims,
	__global int *offsets,
	int root,
	__global char *bvh,
	__global char *shaders,
	__global char *channels,
	int nl,
	__global char *lights,
	__global int *lightOffsets,
	__global float *cumLightPower,
	int skyOffset,
	// sensors
	int ns,
	__global Sensor *sensors,
	int sensor_root,
	__global BVHNode *sensorBvh,
	// params
	int depth,
	float minPower,
	SphereVolume bounds,
	__global MeasurementSensitivityCurve *sensitivityCurves,
	int seed
	)
{
	unsigned int idx = get_global_id(0);
	
	if( idx >= nthreads )
		return;
		
	idx += sampleOffset;
	
	Random rnd;
	initRandom( &rnd, idx, seed );
	
	// spectrum sampling
	Spectrum spectrum;
	SampleUnitSpectum( &spectrum, &rnd );
	//SampleSpectrum( &spectrum, &rnd );
	
	// select a light source proportional to light power
	// all threads are stratified over all light sources
	float cumpower = (float)(idx + random1f(&rnd)) / (float)nsamples;
	
	float lprob; // sample probabilitiy
	__global Light* light = sampleLightSource( &lprob, cumpower, nl, lights, lightOffsets, cumLightPower );
	
	Ray r;
	Spectrum rad;

	// generate a random ray from that light source
	GenerateLight(DEBUG_ARG, &r, &rad, &spectrum, light, &rnd, &bounds, lights, lights );

	// correct for light selection probability
	specsmul( &rad, &rad, invSafe(lprob) );
	
	// bounce until the maximum depth is reached
	for(int d=0; d<=depth; d++)
	{
		// compute first intersection between ray and scene
		Intc intc;
		intc_init( &intc , FLT_MAX , 0);
		
		trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
		
#ifdef ENABLE_SENSORS
		// find all sensors on the unintersected line segment and accumulate irradiance
		traceSensor( DEBUG_ARG, irradiance, detectors, sensitivityCurves, d, measurementBits, &rad, &spectrum, intc.t, &r, ns, sensors, sensorBvh, sensor_root );
#endif
		
		// stop when the ray missed the scene
		if( intc.prim == 0 )
			return;
		
		const __global Prim *prim = intc.prim;
		
		// load intersection data
		Environment env;
		computeIntersectEnv( DEBUG_ARG, &env, &spectrum, &intc, &r, shaders );
				
		Vec3 refl_out;
		Spectrum refl_bsdf;
		
		// sample outgoing direction
		SampleBSDF( &refl_out, &refl_bsdf, &spectrum, &env, &r.d, channels, true, &rnd );

		// compute new radiance after reflection
		Spectrum new_rad;
		specmul( &new_rad, &rad, &refl_bsdf );
				
		// compute absorbed power
		Spectrum absorbed;
		specsub( &absorbed, &rad, &new_rad );

		// don't clamp the absorbed power! 
		// due to probabilistic nature of BSDF sampling, the absorbed power may be negative
		// clamping would bias the net absorbed power
				
		//specone( &absorbed );
		//specsmul( &absorbed, &absorbed, 100.f );
				
		// accumulate absorbed power
		int measurementIdx = GetMeasurementIdx( detectors, d, measurementBits, prim->group_idx );
		AtomicAddSpectrum( &power[measurementIdx], &absorbed, &spectrum, sensitivityCurves );
		//AddSpectrum( &power[measurementIdx], &absorbed, &spectrum, sensitivityCurves );
		
		// check if radiance power is big enough for reflection
		if( specsum( &new_rad ) < minPower )
			break;

#ifdef RUSSIAN_ROULETTE
		if( depth >= RUSSIAN_ROULETTE_DEPTH )
		{
			// Russian roulette performs importance sampling with respect to path length
			
			// compute reflection probability
			float pbrRussian = clamp( specsum( &new_rad ) / SPECTAL_CHANNELS, 0.f, 1.f);
			
			// russian roulette
			if( random1f( &rnd ) >= pbrRussian )
				break;
			
			// correct radiance accordingly
			specsmul( &new_rad, &new_rad, invSafe(pbrRussian) );
		}
#endif	
			
		// set reflection ray
		rad = new_rad;		
		rinit( &r, &env.p, &refl_out );
		rmarch( &r.o, SCATTER_EPSILON, &r );
	}
}
