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
#include "shader/evalmaxbsdf.h"
#include "shader/samplemaxbsdf.h"

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
	int depth,
	float minPower
	)
{
	unsigned int idx = get_global_id(0);
	
	if( idx < nsamples )
	{
		// get correct pixel
		idx += pixelOffset;
		idx = idx % (width * height);
		
		// Generate primary ray
		
		const int ix = (idx % width);
		const int iy = (int)(idx / width);
		
		const int pixel = iy * width + ix;
		const float x = (ix + 0.5f) * 2.f / (float)width - 1.f;
		const float y = (iy + 0.5f) * 2.f / (float)height - 1.f;
				
		Spectrum fX;
		speczero( &fX );
				
		// spectrum sampling
		Spectrum spectrum;
		specone(&spectrum);
				
		Ray r;
		initPrimaryRay( &r, camera, x, y );
		
		Spectrum rad;
		specone( &rad );

		bool hit = false;

		for(int d=0; d<depth; d++)
		{
			Intc intc;
			intc_init( &intc , FLT_MAX , 0);
			
			trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
				
			if( intc.prim == 0 )
			{
				if( skyOffset != -1 )
				{
					// get sky
					__global EnvironmentLight* sky = (__global EnvironmentLight*)(&lights[ skyOffset ]);
					
					// evaluate environment map
					Spectrum bsdf;
					EvalEnvironmentLight( &bsdf, &spectrum, &r.d, &r.o, sky, lights, lights );
					
					// add environment map contribution
					specmad( &fX, &bsdf, &rad, &fX );
					
					hit = true;
					
					// clear radiance
					speczero( &rad );
				}
				break;
			}

			hit = true;

			// compute intersection environment
			Environment env;
			computeIntersectEnv( DEBUG_ARG, &env, &spectrum, &intc, &r, shaders );
			
			// init deterministic random generator;
			Random rnd;
			initRandom( &rnd, 0, 0 );
			
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
					if( EvaluateDiffuseMaxBSDF( &bsdf, &spectrum, &env, &r.d, &sr.d, channels, false ) )
					{
						if( specsum( &bsdf ) > EPSILON )
						{
							Spectrum trans;
							//specone(&trans);
							if( connectMax( DEBUG_ARG, &trans, &spectrum, sr, intc, np, ninfp, prims, offsets, root, bvh, shaders, channels, depth, minPower ) )
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
			
			Vec3 refl_out, trans_out;
			Spectrum refl_bsdf, trans_bsdf;
			
			// sample directions of maximum reflaction and refraction
			SampleMaxBSDF( &refl_out, &refl_bsdf, &trans_out, &trans_bsdf, &spectrum, &env, &r.d, channels, false );
						
			specmul( &rad, &rad, &trans_bsdf );
			
			// use the refraction only
			if( specsum( &rad ) < minPower )
				break;
				
			rinit( &r, &env.p, &trans_out );
			rmarch( &r.o, SCATTER_EPSILON, &r );
		};

		Vec3 rgb;
		Spectral2RGB( &rgb , &fX, &spectrum );
		v3clamp( &rgb, &rgb );
		
		float4 color;
		if( hit )
			//color = (float4)(0.0f,0.f,0.0f,0.f);
			color = (float4)(rgb.x,rgb.y,rgb.z,1.f);
		else
			color = (float4)(0.f,0.f,0.f,1.f);
			//color = (float4)(0.1f,0.f,0.1f,0.f);
		output[pixel] = color;
	}
}
