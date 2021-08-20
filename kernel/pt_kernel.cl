#define GPU_KERNEL

#define INTERPOLATE_NORMALS
#define REFRACTION

//#define RUSSIAN_ROULETTE
		
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

#include "light/samplelight.h"
#include "light/evallight.h"

#include "common/connect.h"
#include "common/intersectenv.h"

__kernel void computeImage( DEBUG_PAR ,
	__global float4 *output , uint width , uint height , 
	uint nsamples, uint pixelOffset ,
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
	int seed,
	int depth,
	float minPower
	)
{
	unsigned int idx = get_global_id(0);
	
	Random rnd;
	initRandom( &rnd, idx, seed );
	
	// spectrum sampling
	Spectrum spectrum;
	SampleSpectrum( &spectrum, &rnd );
	
	if( idx < nsamples )
	{
		// get correct pixel
		idx += pixelOffset;
		idx = idx % (width * height);
		
		// Generate primary ray
		const float pixelSize = 2.f / (float)width;
		
		const int ix = (idx % width);
		const int iy = (int)(idx / width);
		
		float2 rnd_pixel = random2f( &rnd );
		
		const int pixel = iy * width + ix;
		const float x = (ix + rnd_pixel.x) * 2.f / (float)width - 1.f;
		const float y = (iy + rnd_pixel.y) * 2.f / (float)height - 1.f;
				
		Ray r;
		initPrimaryRay( &r, camera, x, y );
		
		Spectrum fX, rad;
		speczero( &fX );
		specone( &rad );
		
		bool specular = true;
		for(int d=0; d<depth; d++)
		{
			Intc intc;
			intc_init( &intc , FLT_MAX , 0);
			
			trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
				
			if( intc.prim == 0 )
			{
				
				if( skyOffset != -1 && specular == true )
				{
					// get sky
					__global EnvironmentLight* sky = (__global EnvironmentLight*)(&lights[ skyOffset ]);
					
					// evaluate environment map
					Spectrum bsdf;
					EvalEnvironmentLight( &bsdf, &spectrum, &r.d, &r.o, sky, lights, lights );
					
					// add environment map contribution
					specmad( &fX, &bsdf, &rad, &fX );
				}
				
				break;
			}

			//const __global Prim *prim = intc.prim;				
			
			// compute intersection environment
			Environment env;
			computeIntersectEnv( DEBUG_ARG, &env, &spectrum, &intc, &r, shaders );
		
			// evaluate all light sources
			Spectrum lum;
			speczero( &lum );
			for( int i = 0 ; i < nl ; i++ )
			{
				__global Light* light = (__global Light*)(&lights[ lightOffsets[i] ]);
			
				// evaluate all samples per light source
				int ns = light->samples;
				float pns = 1.f / (float)ns;
				for( int j = 0 ; j < ns ; j++ )
				{
					Spectrum lb;
					float length;
					
					// get light ray and radiance for this light source
					Ray sr;
					SampleLight( &sr, &length, &lb, &spectrum, &env.p, light, &rnd, lights, lights );
					
					// correct for the number of samples per light source
					specsmul( &lb ,&lb, pns );
						
					Intc intc;
					intc_init( &intc , length , 0 );
					
					// evaluate surface BSDF
					Spectrum bsdf;
					//specone( &bsdf );
					if( EvaluateDiffuseBSDF( &bsdf, &spectrum, &env, &r.d, &sr.d, channels, false ) )
					{
						if( specsum( &bsdf ) > EPSILON )
						{
							Spectrum trans;
							//specone( &trans );
							if( connect( DEBUG_ARG, &trans, &spectrum, sr, intc, np, ninfp, prims, offsets, root, bvh, shaders, channels, depth, minPower ) )
							{
								specmul( &trans ,&trans, &bsdf );
								// accumulate direct light
								specmad( &lum , &lb , &trans , &lum );
							}
						}
					}
				}
			}
			
			specmad( &fX, &lum, &rad, &fX );
			
			Vec3 refl_out;
			Spectrum refl_bsdf;
			
			// sample outgoing direction
			specular &= SampleBSDF( &refl_out, &refl_bsdf, &spectrum, &env, &r.d, channels, false, &rnd );
			
			// scale radiance
			specmul( &rad, &rad, &refl_bsdf );
			
			// check if radiance power is big enough for reflection
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
		};
		
		Vec3 rgb;
		Spectral2RGB( &rgb , &fX, &spectrum );
		v3clamp( &rgb, &rgb );
		
		float4 old_color = output[pixel];
		float4 color = (float4)(old_color.x + rgb.x,old_color.y + rgb.y,old_color.z + rgb.z,1.f);
		output[pixel] = color;
	}
}
