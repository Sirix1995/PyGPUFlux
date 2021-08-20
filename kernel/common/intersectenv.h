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

#ifndef _INTERSECT_ENV_H
#define _INTERSECT_ENV_H

#include "geo/normaluv.h"
#include "shader/switchshader.h"
#include "shader/iorshader.h"
#include "shader/shaderenv.h"

void computeIntersectEnv( DEBUG_PAR ,
	Environment *env,
	const Spectrum *spectrum,
	const Intc *intc,
	const Ray *r,
	const __global char *shaders)
{
	// load intersection data
	rmarch( &env->p , intc->t , r );
		
	// get primitive
	const __global Prim *prim = intc->prim;
	
	// transform intersection point to object space
	Mat34 m = prim->m;
	m34submul( &env->lp , &m , &env->p );
	
	// get normal and uv coordinates
	computePrimitiveNormalUV( DEBUG_ARG, &env->norm, &env->uv, intc, &env->p, prim );

	// set primitive IOR
	env->ior = prim->IOR;
	
	// side switch shader
	const __global Shader *shader = evalSwitchShader( prim->shader_offset, shaders, &r->d, &env->norm );
	
	// ior shader
	env->shader = evalIORShader( shader, &env->ior, spectrum, shaders );
	
	// compute normal and iorRatio such that it points to the same side as the incomming ray
	if (v3dot (&r->d, &env->norm) > 0)
	{
		v3neg( &env->norm, &env->norm );
		(env->ior) = (IORRatio)(1.f) / (env->ior);
	}
	
	//invert ratio from inner/outer to outer/inner to conform with light travel direction
	(env->ior) = (IORRatio)(1.f) / (env->ior);
}

#endif