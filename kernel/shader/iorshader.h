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

#ifndef _IOR_SHADER_H
#define _IOR_SHADER_H

#include "shader/shader.h"
#include "shader/fresnel.h"
#include "color/spectrum.h"

typedef struct
{
	int type;
	float iorA, iorB; // Cauchy's quadratic equation 
	ShaderHandle inputShader; // input shader
} IORShader;
	
inline const __global Shader* evalIORShader( const __global Shader* shader, IORRatio *iorRatio, const Spectrum *spectrum, const __global char *shaders )
{
	// if ior shader
	if( shader->type == SHADER_IOR )
	{
		// get ior shader
		const __global IORShader *iorshader = (const __global IORShader *)(shader);
		
		IORRatio spec;
		
#ifdef REFRACTION
	#ifdef SPECTRUM_DISPERSION
		spec = ((*spectrum));
	#else
		spec = (SPECTRAL_WAVELENGTH_DEFAULT);
	#endif
#else
		spec = ((*spectrum));
#endif

		// compute wavelenght per micrometer
		spec /= 1000.f;
		
		// compute spectral dependent ior ratio using Cauchy's equation with 2 coefficients
		*iorRatio = (IORRatio)(iorshader->iorA) + (IORRatio)(iorshader->iorB) / (spec*spec);
	/*
#ifdef REFRACTION
	#ifdef SPECTRUM_DISPERSION
		// get spectral dependent ior ratio using Cauchy's equation with 2 coefficients
		*iorRatio = iorshader->iorA + iorshader->iorB / ((*spectrum)*(*spectrum));
	#else
		// get default ior ratio
		*iorRatio = iorshader->iorA + iorshader->iorB / (SPECTRAL_WAVELENGTH_DEFAULT*SPECTRAL_WAVELENGTH_DEFAULT);
	#endif
#else
	// get spectral dependent ior ratio using Cauchy's equation with 2 coefficients
	*iorRatio = iorshader->iorA + iorshader->iorB / ((*spectrum)*(*spectrum));
#endif
	
#ifdef SPECTRUM_DISPERSION
		// get spectral dependent ior ratio using Cauchy's equation with 2 coefficients
		*iorRatio = iorshader->iorA + iorshader->iorB / ((*spectrum)*(*spectrum));
#else
		// get default ior ratio
		*iorRatio = iorshader->iorA + iorshader->iorB / (SPECTRAL_WAVELENGTH_DEFAULT*SPECTRAL_WAVELENGTH_DEFAULT);
#endif
*/
		// set input shader
		return (const __global Shader*)&shaders[iorshader->inputShader];
	}
			
	return shader;
}

#endif

