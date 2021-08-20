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

/*
 * Measured results are accumulated synchronized in the measurement buffers.
 * OpenCL does not natively support atomic floating points operations, so they are emulated using the available integer atomics.
 * Possible apporaches:
 * 	  1. Atomicaly accumulate each individual float using a spinadd with atomic integer exchange(add two floats for 64-bit atomics)
 *	  2. Aquire a critical section on the relevant buffer part and update all floats
 *    3. Store the floats in fixed point and use integer atomic add
 * Option 1 and 2 do not require java support but require active spin
 * Option 3 has the best native support but requires fixed-point precision fine tuning
 * Option 2 scales best for larger vectors or double-precision measurements, but requires special attention to prevent deadlock
 * For now, options 1 and 2 are implemented. If both appoaches give problems, option 3 can be used as a last resort.
 * For comparison, a non-synchronized version is also provided to access the cost of synchronization and for giving an upperbound on perfornace.
 */

#ifndef _SYNC_H
#define _SYNC_H

#define CRITIAL_SECTION_ADD

#include "math/vec3.h"

#if 1 && defined(CL_KHR_GLOBAL_INT32_BASE_ATOMICS) 

	#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable
		
	void AtomicAdd(__global float *val, const float delta) {
		union {
			float f;
			unsigned int i;
		} oldVal;
		union {
			float f;
			unsigned int i;
		} newVal;

		do {
			oldVal.f = *val;
			newVal.f = oldVal.f + delta;
		} while (atom_cmpxchg((__global unsigned int *)val, oldVal.i, newVal.i) != oldVal.i);
	}
	
	/*
	#if 1 && !defined(CRITIAL_SECTION_ADD)

		inline void AtomicAddf1( volatile __global float *value, float f )
		{
			// load current value
			float old_val = *value, curr_val, new_val;
			
			do
			{
				// get current value
				curr_val = old_val;
				// compute new value
				new_val = curr_val + f;
				// exchange iff value has not changed since loading
				old_val = as_float( atom_cmpxchg( (volatile __global int*)value, as_int( curr_val ),  as_int( new_val ) ) );
			// repeat until exchange succeded
			}while( old_val != curr_val );
		};

		#if defined(CL_KHR_GLOBAL_INT64_BASE_ATOMICS)

			#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

			inline void AtomicAddf2( volatile __global float2 *value, float2 f )
			{
				// load current value
				float2 old_val = *value, curr_val, new_val;
				
				do
				{
					// get current value
					curr_val = old_val;
					// compute new value
					new_val = curr_val + f;
					// exchange iff value has not changed since loading
					old_val = as_float2( atom_cmpxchg( (volatile __global long*)value, *((long*)&curr_val),  *((long*)&new_val) ) );
				// repeat until exchange succeded
				}while( old_val != curr_val );
			};

		#else

			inline void AtomicAddf2( volatile __global float2 *value, float2 f )
			{
				AtomicAddf1( ((volatile __global float *)value) + 0, f.x );
				AtomicAddf1( ((volatile __global float *)value) + 1, f.y );
			};

		#endif

		inline void AtomicAddVec3( volatile __global float4 *value, Vec3 *f )
		{
			AtomicAddf2( (volatile __global float2 *)value, (float2)(f->x,f->y) );
			AtomicAddf1( ((volatile __global float *)value) + 2, f->z );
		};

	#else

		inline void AtomicAddVec3( __global float4 *value, Vec3 *f )
		{
			// get critical section flag
			__global int *cs = ((__global int*)(value)) + 3;
			
			int ret_val;
			
			// repeat
			do
			{
				// try to aquire critical section
				ret_val = atom_xchg( cs, 1 );
				
				// if critial section was free
				if( ret_val == 0 )
				{
					// accumulate value
					*value += (float4)(f->x,f->y,f->z,0.f); 
					
					// force writes
					write_mem_fence( CLK_GLOBAL_MEM_FENCE );
					
					// free critial section
					*cs = 0;
				}

				// force local writes and reads
				mem_fence( CLK_LOCAL_MEM_FENCE );
				
				// continue if critical section was in use
			}while( ret_val == 1 );
		};

	#endif
	*/
#else

	void AtomicAdd(__global float *val, const float delta) {
		*val += delta;
	}
	
/*
	inline void AtomicAddVec3( __global float4 *value, Vec3 *f )
	{
		*value += (float4)(f->x,f->y,f->z,0.f); 
	};*/

#endif
	

#endif