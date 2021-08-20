#define GPU_KERNEL

#define INTERPOLATE_NORMALS
#define REFRACTION

#include "util/debug.h"

#include "common/settings.h"

#include "math/mat34.h"
#include "math/vec2.h"
#include "math/math.h"

#include "common/camera.h"

#include "geo/intersect.h"
#include "geo/normaluv.h"

#include "trace/trace.h"

#include "shader/shader.h"
#include "shader/evalbsdf.h"
#include "shader/samplebsdf.h"
#include "shader/samplemaxbsdf.h"

#include "light/genlight.h"

#include "math/probability.h"

#include "common/connect.h"
#include "common/intersectenv.h"

__kernel void computeImage( DEBUG_PAR ,
	__global float4 *output , uint nrays , uint sampleOffset ,
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
	__constant Camera *camera,
	SphereVolume bounds,
	int seed,
	int depth,
	float minPower
	)
{
	unsigned int idx = get_global_id(0);
	
	if( idx >= nrays )
		return;
	
	int ridx = idx + sampleOffset;
	
	// init randomizer
	Random rnd;
	initRandom( &rnd, ridx, seed );
	
	// spectrum sampling
	Spectrum spectrum;
	SampleUnitSpectum( &spectrum, &rnd );
	//SampleSpectrum( &spectrum, &rnd );
	
	// compute cumulative light power
	float cumpower = (float)(idx + random1f(&rnd)) / (float)nrays;
	
	// get light source
	float lprob; // sample probabilitiy
	__global Light* light = sampleLightSource( &lprob, cumpower, nl, lights, lightOffsets, cumLightPower );
	
	Ray r;
	Spectrum rad;
	
	// generate random photon
	GenerateLight( DEBUG_ARG, &r, &rad, &spectrum, light, &rnd, &bounds, lights, lights );

	// correct for light selection probability
	specsmul( &rad, &rad, invSafe(lprob) );
	
	// bounce until the maximum depth is reached
	for(int d=0; d<depth; d++)
	{
		// compute first intersection between ray and scene
		Intc intc;
		intc_init( &intc , FLT_MAX , 0);
		
		trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
				
		if( intc.prim == 0 )
			return;
				
		//const __global Prim *prim = intc.prim;
		
		// compute intersection environment
		Environment env;
		computeIntersectEnv( DEBUG_ARG, &env, &spectrum, &intc, &r, shaders );
		
		// project on image plane
		Vec2 sp;
		Vec3 projp;
		if( projectOnImage( &sp, &projp, &env.p, camera ) )
		{
			float factor = PrbAreaToImage( &env.p, &env.norm, camera );
						
			// construct connection ray
			Vec3 d;
			v3sub( &d , &projp , &env.p );
			
			float length = v3len( &d );
			v3smul( &d , &d , 1.f / length );
			
			Ray sr;
			rinit( &sr , &env.p , &d );
			rmarch( &sr.o , 0.001f , &sr );
		
			length -= 0.002f;
			Intc sintc;
			intc_init( &sintc , length , 0);
			
			// compute projection term
			const float idot = v3dot( &r.d , &env.norm );
			const float odot = v3dot( &sr.d , &env.norm );
						
			// evaluate bsdf
			Spectrum bsdf;
			specone( &bsdf );
			if( EvaluateDiffuseBSDF( &bsdf, &spectrum, &env, &r.d, &sr.d, channels, true ) )
			{
				if( specsum( &bsdf ) > EPSILON )
				{
					// contribute to the correct pixel
					specmul( &bsdf, &bsdf, &rad);
				
					Spectrum lum;
					specone( &lum );
					if( connect( DEBUG_ARG, &lum, &spectrum, sr, sintc, np, ninfp, prims, offsets, root, bvh, shaders, channels, depth, minPower ) )
					{
						specmul( &lum, &lum, &bsdf);
						// correct for projection to image space and the projection in the BSDF
						specsmul( &lum, &lum, invSafe(fabs(odot) * factor) );
						
						sp.x = (sp.x + 1.f) / 2;
						sp.y = (sp.y + 1.f) / 2;
						int pixel = (int)(sp.x * camera->width) + (int)(sp.y * camera->height) * camera->width;
						
						if( pixel >= 0 && pixel < camera->width * camera->height )
						{
							Vec3 rgb;
							Spectral2RGB( &rgb , &lum, &spectrum );
							
							v3clamp( &rgb, &rgb );
							
							float4 old_color = output[pixel];
							float4 color = (float4)(old_color.x + rgb.x,old_color.y + rgb.y,old_color.z + rgb.z,1.f);
							output[pixel] = color;
						}
					}
				}
			}
		}
		
		Vec3 refl_out;
		Spectrum refl_bsdf;
		
		// sample outgoing direction
		SampleBSDF( &refl_out, &refl_bsdf, &spectrum, &env, &r.d, channels, true, &rnd );
					
		specmul( &rad, &rad, &refl_bsdf );

		// use the refraction only
		if( specsum( &rad ) < minPower )
			break;
		
#ifdef RUSSIAN_ROULETTE
		if( depth >= RUSSIAN_ROULETTE_DEPTH )
		{
			// Russian roulette performs importance sampling with respect to path length
			
			// compute reflection probability
			float pbrRussian = clamp( specsum( &rad ) / SPECTAL_CHANNELS, 0.f, 1.f);
			
			// russian roulette
			if( random1f( &rnd ) >= pbrRussian )
				break;
			
			// correct radiance accoardingly
			specsmul( &rad, &rad, invSafe(pbrRussian) );
		}
#endif
			
		rinit( &r, &env.p, &refl_out );
		rmarch( &r.o, SCATTER_EPSILON, &r );
	}
	
}
