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

#ifndef _BVH_TRACE_H
#define _BVH_TRACE_H

#include "trace/bvh/bvh.h"
#include "geo/intersect.h"
#include "util/util.h"

#define RAY_RESULT_POS		63		// steal one entry from stack (saves a register)
#define ENTRYPOINT_SENTINEL 0x76543210

void bvhTrace( DEBUG_PAR,
	Intc *intc, 
	const Ray *r,
	const RayAux *aux,
	const __global char *prims,
	const __global int *offsets,
	const __global BVHNode *bvh,
	int root,
	bool shadowRay )
{
	int traversalStack[RAY_RESULT_POS+1];
	
	//-----------------------------------------------------
	// Traversal init
	//-----------------------------------------------------

	traversalStack[0] = ENTRYPOINT_SENTINEL;	// init traversal stack
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
			// Fetch 2 child nodes (boxes + header)
			const float4 n0xy 	= bvh[nodeAddr].n0xy;	// node0: x0,x1,y0,y1
			const float4 nz		= bvh[nodeAddr].nz;		// node0: z0,z1, node1: z0,z1
			const float4 n1xy 	= bvh[nodeAddr].n1xy;	// node1: x0,x1,y0,y1
			const int4 	 cnodes = bvh[nodeAddr].cnodes;
			
			// Perform 2 ray-box tests
			const float x0Child0 = n0xy.x * aux->idir.x - aux->ood.x;
			const float x1Child0 = n0xy.y * aux->idir.x - aux->ood.x;
			const float y0Child0 = n0xy.z * aux->idir.y - aux->ood.y;
			const float y1Child0 = n0xy.w * aux->idir.y - aux->ood.y;
			const float z0Child0 = nz.x   * aux->idir.z - aux->ood.z;
			const float z1Child0 = nz.y   * aux->idir.z - aux->ood.z;
			const float z0Child1 = nz.z   * aux->idir.z - aux->ood.z;
			const float z1Child1 = nz.w   * aux->idir.z - aux->ood.z;
			float tminChild0 = max(0.f,    min(x0Child0, x1Child0));
			float tmaxChild0 = min(intc->t,max(x0Child0, x1Child0));
			float tminChild1 = max(0.f,    min(z0Child1, z1Child1));
			float tmaxChild1 = min(intc->t,max(z0Child1, z1Child1));
			tminChild0 = max(tminChild0, min(y0Child0, y1Child0));
			tmaxChild0 = min(tmaxChild0, max(y0Child0, y1Child0));
			tminChild0 = max(tminChild0, min(z0Child0, z1Child0));
			tmaxChild0 = min(tmaxChild0, max(z0Child0, z1Child0));
			float x0Child1 = n1xy.x * aux->idir.x - aux->ood.x;
			float x1Child1 = n1xy.y * aux->idir.x - aux->ood.x;
			float y0Child1 = n1xy.z * aux->idir.y - aux->ood.y;
			float y1Child1 = n1xy.w * aux->idir.y - aux->ood.y;
			tminChild1 = max(tminChild1, min(y0Child1, y1Child1));
			tmaxChild1 = min(tmaxChild1, max(y0Child1, y1Child1));
			tminChild1 = max(tminChild1, min(x0Child1, x1Child1));
			tmaxChild1 = min(tmaxChild1, max(x0Child1, x1Child1));

			const int traverseChild0 = (tmaxChild0 >= tminChild0);
			const int traverseChild1 = (tmaxChild1 >= tminChild1);

			//------------------------------------------------------
			// Traversal decision
			// - this organization compiled to the fastest code
			//------------------------------------------------------

			nodeAddr		   = cnodes.x;		// stored as int
			int nodeAddrChild1 = cnodes.y;		// stored as int

			if(traverseChild0 != traverseChild1)
			{
				if(traverseChild1)
					nodeAddr = nodeAddrChild1;
			}
			else
			{
				if(!traverseChild0)	// Neither
				{
					nodeAddr = traversalStack[traversalStackPtr];
					--traversalStackPtr;
				}
				else				// Both
				{
					if(tminChild1 < tminChild0)					// Ensure Child0 is near, Child1 is far
						swapInt( &nodeAddr , &nodeAddrChild1 );

					++traversalStackPtr;								// a separate statement thanks to 2.1 compiler shortcoming
					traversalStack[traversalStackPtr] = nodeAddrChild1;	// push far
				}
			}
		}

		//------------------------------------------------------
		// Enter leaf
		//------------------------------------------------------

		if(nodeAddr<0)
		{
			// Fetch node header
			int4 leaf = bvh[(-nodeAddr-1)].cnodes;
			int primAddr  = leaf.x;					// stored as int
			int primCount = leaf.y;					// stored as int
			
			//---------------------------------------------------------
			// Ray-primitive intersections
			//---------------------------------------------------------
			
			if( computeIntersect( DEBUG_ARG, intc, primAddr, primCount, prims, offsets, r, aux, shadowRay ) )
			{
				// Terminate for shadow ray intersection
				return;
			}

			// POP stack
			nodeAddr = traversalStack[traversalStackPtr];
			--traversalStackPtr;
		}
	}
}

#endif