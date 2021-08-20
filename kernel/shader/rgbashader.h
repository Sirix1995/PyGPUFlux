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

#ifndef _RGBA_SHADER_H
#define _RGBA_SHADER_H

#include "color/color.h"
#include "shader/shader.h"
#include "shader/fresnel.h"

typedef struct
{
	Shader base;
	
	Vec3 color;
	float alpha;
} RGBAShader;

inline void EvalRGBAEnv(
	Spectrum *refl, Spectrum *trns,
	const Spectrum *spectrum,
	const __global RGBAShader *rgba,
	const IORRatio *fresnel, const IORRatio *ior, bool adjoint )
{
	Spectrum trans = (1.f - rgba->alpha) * (1.f - (*fresnel));

#ifdef REFRACTION
	if (!adjoint)
	{
		// correction of BSDF as described by Eric Veach in his thesis
		trans *= (*ior) * (*ior);
	}
#endif

	Vec3 rgb = rgba->color;
	Spectrum color;
	RGB2Spectral( &color, &rgb , spectrum );
	
	// set transparancy
	//*trns = color * trans;
	*trns = trans;
	
	// interpolate transparancy for energy conservation
	*refl = color * (1.f - trans);
	//specsmul( refl, &color, 1.f - trans  );

/*
	// compute the transmission coefficient
	//float trans = (1 - rgba->alpha) * (1 - fresnel);
	
	float trans = (1 - rgba->alpha) ;
		
#ifdef REFRACTION
	if (!adjoint)
	{
		// correction of BSDF as described by Eric Veach in his thesis
		trans *= (*ior) * (*ior);
	}
#endif

	trans *= (*fresnel);

	Vec3 rgb = rgba->color;
	Spectrum color;
	RGB2Spectral( &color, &rgb , spectrum );
	
	// set transparancy
	specsmul( trns, &color, trans );
	
	// interpolate transparancy for energy conservation
	specsmul( refl, &color, 1.f - trans  );
*/
}

#endif
