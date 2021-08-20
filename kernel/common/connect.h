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

#ifndef _CONNECT_H
#define _CONNECT_H

#include "geo/normaluv.h"

#include "trace/trace.h"

#include "shader/evalbsdf.h"
#include "shader/evalmaxbsdf.h"

#include "common/intersectenv.h"

bool connect( DEBUG_PAR, 
	Spectrum *rad, 
	const Spectrum *spectrum, 
	Ray r, Intc intc, 
	int np , int ninfp ,
	const __global char *prims,
	const __global int *offsets,
	int root,
	const __global char *bvh,
	const __global char *shaders,
	const __global char *channels,
	int depth,
	float minPower )
{
	// contribute to the correct pixel
	specone( rad );
	
	// get length
	float length = intc.t;

	for( int k = 0; k < depth; k++ )
	{
		// trace shadow ray
		trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
		
		// if connection is clear
		if( intc.prim == 0 )
		{
			return true;
		}
		else
		{
			
			//const __global Prim *s_prim = intc.prim;
			
			// compute intersection environment
			Environment env;
			computeIntersectEnv( DEBUG_ARG, &env, spectrum, &intc, &r, shaders );
						
			// evaluate transparancy
			Spectrum trans;
			if( !EvaluateSpecularBSDF( &trans, spectrum, &env, &r.d, &r.d, channels, false ) )
				break;
							
			// accumulate transparancy
			specmul( rad, rad, &trans );
			
			// advance shadow ray
			rmarch( &r.o , intc.t + CONNECT_EPSILON , &r );
			length -= (intc.t + 2*CONNECT_EPSILON);
			intc_init( &intc , length , 0 );
			
		}
		
		if( specsum( rad ) < minPower )
			break;
	}
	
	return false;
}


bool connectMax( DEBUG_PAR, 
	Spectrum *rad, 
	const Spectrum *spectrum, 
	Ray r, Intc intc, 
	int np , int ninfp ,
	__global char *prims,
	__global int *offsets,
	int root,
	__global char *bvh,
	__global char *shaders,
	__global char *channels,
	int depth,
	float minPower )
{
	// contribute to the correct pixel
	specone( rad );
		
	// get length
	float length = intc.t;

	for( int k = 0; k < depth; k++ )
	{
		// trace shadow ray
		trace( DEBUG_ARG, &intc, &r, np, ninfp, prims, offsets, bvh, root, false );
		
		// if connection is clear
		if( intc.prim == 0 )
		{
			return true;
		}
		else
		{
			// compute intersection environment
			Environment env;
			computeIntersectEnv( DEBUG_ARG, &env, spectrum, &intc, &r, shaders );
			
			// advance shadow ray to intersection point
			rmarch( &r.o , intc.t + CONNECT_EPSILON , &r );
			length -= (intc.t + 2*CONNECT_EPSILON);
			intc_init( &intc , length , 0 );
			
			// evaluate transparancy
			Spectrum trans;
			
			if( !EvaluateSpecularMaxBSDF( &trans, spectrum, &env, &r.d, &r.d, channels, false ) )
				break;
				
			// accumulate transparancy
			specmul( rad, rad, &trans );
		}
		
		if( specsum( rad ) < minPower )
			break;
	}
	
	return false;
}

#endif