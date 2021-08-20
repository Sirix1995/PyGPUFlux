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

#ifndef _TRACE_H
#define _TRACE_H

#define SPATIAL_STRUCTURE

#ifdef BVH
	#include "trace/bvh/trace.h"
#else
	#include "trace/bih/trace.h"
#endif

#include "geo/intersect.h"
#include "util/util.h"

void trace( DEBUG_PAR,
	Intc *intc, 
	const Ray *r,
	int np , int ninfp ,
	const __global char *prims,
	const __global int *offsets,
	const __global char *bvh,
	int root,
	bool shadowRay )
{
	RayAux aux;
	raux( &aux, r );
	
	if( computeIntersect( DEBUG_ARG , intc, 0, ninfp, prims, offsets, r, &aux, shadowRay ) )
		return;

#ifdef SPATIAL_STRUCTURE
	#ifdef BVH
		bvhTrace( DEBUG_ARG, intc, r, &aux, prims, offsets, (__global BVHNode*)bvh, root, shadowRay );
	#else
		bihTrace( DEBUG_ARG, intc, r, &aux, prims, offsets, (__global BIHNode*)bvh, root, shadowRay );
	#endif
#else
	computeIntersect( DEBUG_ARG , intc, ninfp, np, prims, offsets, r, &aux, shadowRay );
#endif

}

#endif