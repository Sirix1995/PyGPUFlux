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
 
/*
	This file is derived from the "persistent while-while kernel" used in

	"Understanding the Efficiency of Ray Traversal on GPUs",
	Timo Aila and Samuli Laine,
	Proc. High-Performance Graphics 2009
*/

#ifndef _BIH_TRACE_H
#define _BIH_TRACE_H

#include "trace/bih/bih.h"
#include "geo/intersect.h"
#include "util/util.h"

#define RAY_RESULT_POS		63		// steal one entry from stack (saves a register)
#define ENTRYPOINT_SENTINEL 0x76543210

typedef struct 
{
	float tMin, tMax;
	int nodeAddr;
}Stack;

void bihTrace( DEBUG_PAR,
	Intc *intc, 
	const Ray *r,
	const RayAux *aux,
	const __global char *prims,
	const __global int *offsets,
	const __global BIHNode *bih,
	int root,
	bool shadowRay )
{
	Stack traversalStack[RAY_RESULT_POS+1];
	
	//-----------------------------------------------------
	// Traversal init
	//-----------------------------------------------------

	// TODO: clip against root bbox
	
	traversalStack[0].nodeAddr = ENTRYPOINT_SENTINEL;	// init traversal stack
	float tMin = 0.f, tMax = intc->t;
	int traversalStackPtr = 0;
	int nodeAddr = root;
	
	//------------------------------------------------------
	// TRAVERSE!
	//------------------------------------------------------

	while(nodeAddr!=ENTRYPOINT_SENTINEL)
	{
		//---------------------------------------------
		// Inner node
		//---------------------------------------------

		for(;nodeAddr>=0 && nodeAddr!=ENTRYPOINT_SENTINEL;)
		{
			// get children
			int nearChild = bih[nodeAddr].c0idx;
			int farChild = bih[nodeAddr].c1idx;

			// obtain axis
			int axis = nearChild & 0x3;
			nearChild = nearChild >> 2;
					
			// compute intersections
			
			float o,id;
			switch( axis )
			{
			case 0:
				o = r->o.x; id = aux->idir.x;
			case 1:
				o = r->o.y; id = aux->idir.y;
			case 2:
				o = r->o.z; id = aux->idir.z;
			};
			
			float tNear = (bih[nodeAddr].left - o) * id;
			float tFar = (bih[nodeAddr].right - o) * id;
			
			// swap direction
			if( id < 0.f )
			{
				float ftmp = tNear;
				tNear = tFar;
				tFar = ftmp;
				
				int itmp = nearChild;
				nearChild = farChild;
				farChild = itmp;
			}
			
			const int missBoth = (tMin > tMax);
			const int missNearChild = missBoth | (tMin > tNear);
			const int missFarChild = missBoth | (tMax < tFar);

			if(missNearChild && missFarChild)
			{
				// POP stack
				 nodeAddr = traversalStack[traversalStackPtr].nodeAddr;
				 tMin = traversalStack[traversalStackPtr].tMin;
				 tMax = traversalStack[traversalStackPtr].tMax;
				 tMax = min( tMax , intc->t );
				--traversalStackPtr;
			}
			else
			{
				if(!missNearChild)
				{
					if(!missFarChild)
					{
						/* Stack the far node */
						++traversalStackPtr;
						traversalStack[traversalStackPtr].tMin = max(tMin, tFar);
						traversalStack[traversalStackPtr].tMax = tMax;
						traversalStack[traversalStackPtr].nodeAddr = farChild;
					}
					
					tMax       = min(tMax, tNear);
					nodeAddr   = nearChild;
				}
				else
				{
					tMin       = max(tMin, tFar);
					nodeAddr   = farChild;
				}
			}
		}
		
		//------------------------------------------------------
		// Enter leaf
		//------------------------------------------------------
	
		if(nodeAddr < 0)
		{
			// Fetch node header
			int primAddr  = bih[(-nodeAddr-1)].idx;		// stored as int
			int primCount = bih[(-nodeAddr-1)].pcount;	// stored as int
			
			//---------------------------------------------------------
			// Ray-primitive intersections
			//---------------------------------------------------------
			
			if( computeIntersect( DEBUG_ARG, intc, primAddr, primCount, prims, offsets, r, aux, shadowRay ) )
			{
				// Terminate for shadow ray intersection
				return;
			}

			// POP stack
			nodeAddr = traversalStack[traversalStackPtr].nodeAddr;
			tMin = traversalStack[traversalStackPtr].tMin;
			tMax = traversalStack[traversalStackPtr].tMax;
			tMax = min( tMax , intc->t );
			--traversalStackPtr;
		}
		
	}
}

#endif