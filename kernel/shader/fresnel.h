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

#ifndef _FRESNEL_H
#define _FRESNEL_H

#include "math/vec3.h"
#include "color/spectralcolor.h"
	
	/**
	 * Computes reflected and transmitted directions according to
	 * Fresnel's formulas.
	 * 
	 * @param reflectedOut the computed unit vector of the reflection direction
	 * @param transmittedOut the computed unit vector of the transmission direction
	 * @param normal the normal unit vector of the surface
	 * @param in the direction unit vector of the incoming ray
	 * @param iorRatio the index of refraction of the surface side where
	 *   the normal vector points into, divided by the index of refraction
	 *   of the opposite side
	 * @return the reflection coefficient
	 */
	 /*
float Fresnel (Vec3 *reflectedOut, Vec3 *transmittedOut, const Vec3 *normal, const Vec3 *in, float iorRatio )
{
	*transmittedOut = *in;
		
	float cos = -v3dot( normal, in );
	v3smad( reflectedOut, normal, 2 * cos, transmittedOut );

	float tmp = iorRatio * cos;
	float t = (1 - iorRatio * iorRatio) + tmp * tmp;
	if (t <= 0)
	{
		return 1;
	}
	else
	{
		v3smul( transmittedOut, in, iorRatio );
		float cost = (float) sqrt (t);
		v3smad( transmittedOut, normal , (iorRatio * cos - cost) , transmittedOut );
		
		v3norm( transmittedOut , transmittedOut ); // Is this realy necceseary??
		
		tmp = (cost - iorRatio * cos) / (cost + iorRatio * cos);
		float Rs = tmp * tmp;
		tmp = (cos - iorRatio * cost) / (cos + iorRatio * cost);
		float Rp = tmp * tmp;
		
		return (Rs + Rp) * 0.5f;
		}
}
	*/

	/**
	 * Computes reflection coefficient according to
	 * Fresnel's formulas.
	 * 
	 * @param normal the normal unit vector of the surface
	 * @param in the direction unit vector of the incoming ray
	 * @param iorRatio the index of refraction of the surface side where
	 *   the normal vector points into, divided by the index of refraction
	 *   of the opposite side
	 * @return the reflection coefficient
	 */
	 /*
float EvalFresnel (const Vec3 *normal, const Vec3 *in, float iorRatio )
{
	float cos = -v3dot( normal, in );

	float tmp = iorRatio * cos;
	float t = (1 - iorRatio * iorRatio) + tmp * tmp;
	if (t <= 0)
	{
		return 1;
	}
	else
	{
		float cost = (float) sqrt (t);
		
		tmp = (cost - iorRatio * cos) / (cost + iorRatio * cos);
		float Rs = tmp * tmp;
		tmp = (cos - iorRatio * cost) / (cos + iorRatio * cost);
		float Rp = tmp * tmp;
		
		return (Rs + Rp) * 0.5f;
	}
}
	*/
	
#ifdef REFRACTION
	/* When refraction is enabled, we require a unique ior ratio per path*/
	typedef float IORRatio;
#else
	/*When refraction is disabled, we can use an ior ratio for each spectral channel, allowing to simulate multiple channels at once*/
	typedef Spectrum IORRatio;
#endif
	
IORRatio EvalFresnel(const Vec3 *normal, const Vec3 *in, const IORRatio *iorRatio )
{
	float cos = -v3dot( normal, in );

	IORRatio tmp = (*iorRatio) * cos;
	IORRatio t = ((IORRatio)(1.f) - (*iorRatio) * (*iorRatio)) + tmp * tmp;
	
	IORRatio cost = (IORRatio) sqrt (t);
		
	tmp = (cost - (*iorRatio) * cos) / (cost + (*iorRatio) * cos);
	IORRatio Rs = tmp * tmp;
	tmp = (cos - (*iorRatio) * cost) / (cos + (*iorRatio) * cost);
	IORRatio Rp = tmp * tmp;
	
	IORRatio fresnel = (Rs + Rp) * 0.5f;
	fresnel = select( fresnel, (IORRatio)(1.f), (t <= 0) );
	
	return fresnel;
	/*
	if (t <= 0)
	{
		return 1;
	}
	else
	{
		IORRatio cost = (IORRatio) sqrt (t);
		
		tmp = (cost - (*iorRatio) * cos) / (cost + (*iorRatio) * cos);
		IORRatio Rs = tmp * tmp;
		tmp = (cos - (*iorRatio) * cost) / (cos + (*iorRatio) * cost);
		IORRatio Rp = tmp * tmp;
		
		return (Rs + Rp) * 0.5f;
	}
	*/
}

#ifdef REFRACTION
	IORRatio Fresnel (Vec3 *reflectedOut, Vec3 *transmittedOut, const Vec3 *normal, const Vec3 *in, const IORRatio *iorRatio )
	{
		*transmittedOut = *in;
			
		float cos = -v3dot( normal, in );
		v3smad( reflectedOut, normal, 2 * cos, transmittedOut );

		float tmp = (*iorRatio) * cos;
		float t = (1 - (*iorRatio) * (*iorRatio)) + tmp * tmp;
		if (t <= 0)
		{
			return 1.f;
		}
		else
		{
			v3smul( transmittedOut, in, (*iorRatio) );
			float cost = (float) sqrt (t);
			v3smad( transmittedOut, normal , ((*iorRatio) * cos - cost) , transmittedOut );
			
			v3norm( transmittedOut , transmittedOut ); // Is this realy necceseary??
			
			tmp = (cost - (*iorRatio) * cos) / (cost + (*iorRatio) * cos);
			float Rs = tmp * tmp;
			tmp = (cos - (*iorRatio) * cost) / (cos + (*iorRatio) * cost);
			float Rp = tmp * tmp;
			
			return (Rs + Rp) * 0.5f;
		}
	}
	
#else
	IORRatio Fresnel (Vec3 *reflectedOut, Vec3 *transmittedOut, const Vec3 *normal, const Vec3 *in, const IORRatio *iorRatio )
	{
		*transmittedOut = *in;
		
		float cos = -v3dot( normal, in );
		v3smad( reflectedOut, normal, 2 * cos, transmittedOut );
		
		return EvalFresnel( normal, in, iorRatio );
	}
	
#endif

#endif