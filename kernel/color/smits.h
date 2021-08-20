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

#ifndef _SMITS_H
#define _SMITS_H

#include "color/spectralcolor.h"

#define SMITS_WHITE_BASE_FUNC 0
#define SMITS_CYAN_BASE_FUNC 1
#define SMITS_MAGENTA_BASE_FUNC 2
#define SMITS_YELLOW_BASE_FUNC 3
#define SMITS_RED_BASE_FUNC 4
#define SMITS_GREEN_BASE_FUNC 5
#define SMITS_BLUE_BASE_FUNC 6

#define SMITS_MAX_LAMBDA 720.f
#define SMITS_MIN_LAMBDA 380.f

#ifdef SMITS_BINS_LARGE
	#define SMITS_BINS 10
	#define SMITS_SCALE 1.0f
#else
	#define SMITS_BINS 32
	#define SMITS_SCALE 0.94112876f
#endif

#ifdef SMITS_BINS_LARGE
	__constant float smits_lut[7][10] = 	{
		// white
		{1.0000, 	1.0000, 	0.9999, 	0.9993, 	0.9992, 	0.9998, 	1.0000, 	1.0000, 	1.0000, 	1.0000}, 
		// cyan
		{0.9710, 	0.9426, 	1.0007, 	1.0007, 	1.0007, 	1.0007, 	0.1564, 	0.0000, 	0.0000, 	0.0000}, 
		// magenta
		{1.0000, 	1.0000, 	0.9685, 	0.2229, 	0.0000, 	0.0458, 	0.8369, 	1.0000, 	1.0000, 	0.9959}, 
		// yellow
		{0.0001, 	0.0000, 	0.1088, 	0.6651, 	1.0000, 	1.0000, 	0.9996, 	0.9586, 	0.9685, 	0.9840}, 
		// red
		{0.1012,	0.0515, 	0.0000, 	0.0000, 	0.0000, 	0.0000, 	0.8325, 	1.0149, 	1.0149, 	1.0149}, 
		// green
		{0.0000, 	0.0000, 	0.0273, 	0.7937, 	1.0000, 	0.9418, 	0.1719, 	0.0000, 	0.0000, 	0.0025}, 
		// blue
		{1.0000,	1.0000,		0.8916,		0.3323,		0.0000,		0.0000,		0.0003,		0.0369,		0.0483,		0.0496}
	};
#else
	__constant float smits_lut[7][32] =	{
		//WHITE
		{ 1.0618958571272863e+00f,   1.0615019980348779e+00f,	1.0614335379927147e+00f,   1.0622711654692485e+00f,	1.0622036218416742e+00f,   1.0625059965187085e+00f,	1.0623938486985884e+00f,   1.0624706448043137e+00f,		1.0625048144827762e+00f,   1.0624366131308856e+00f,		1.0620694238892607e+00f,   1.0613167586932164e+00f,		1.0610334029377020e+00f,   1.0613868564828413e+00f,		1.0614215366116762e+00f,   1.0620336151299086e+00f,		1.0625497454805051e+00f,   1.0624317487992085e+00f,		1.0625249140554480e+00f,   1.0624277664486914e+00f,		1.0624749854090769e+00f,   1.0625538581025402e+00f,		1.0625326910104864e+00f,   1.0623922312225325e+00f,		1.0623650980354129e+00f,   1.0625256476715284e+00f,		1.0612277619533155e+00f,   1.0594262608698046e+00f,		1.0599810758292072e+00f,   1.0602547314449409e+00f, 1.0601263046243634e+00f,   1.0606565756823634e+00f },
		//CYAN
		{ 1.0414628021426751e+00f,   1.0328661533771188e+00f, 1.0126146228964314e+00f,   1.0350460524836209e+00f,		1.0078661447098567e+00f,   1.0422280385081280e+00f,		1.0442596738499825e+00f,   1.0535238290294409e+00f,		1.0180776226938120e+00f,   1.0442729908727713e+00f,		1.0529362541920750e+00f,   1.0537034271160244e+00f,		1.0533901869215969e+00f,   1.0537782700979574e+00f,		1.0527093770467102e+00f,   1.0530449040446797e+00f,		1.0550554640191208e+00f,   1.0553673610724821e+00f,		1.0454306634683976e+00f,   6.2348950639230805e-01f,		1.8038071613188977e-01f,  -7.6303759201984539e-03f,		-1.5217847035781367e-04f,  -7.5102257347258311e-03f,		-2.1708639328491472e-03f,   6.5919466602369636e-04f,		1.2278815318539780e-02f,  -4.4669775637208031e-03f,		1.7119799082865147e-02f,   4.9211089759759801e-03f,		5.8762925143334985e-03f,   2.5259399415550079e-02f 	},
		//MAGENTA 
		{ 9.9422138151236850e-01f,   9.8986937122975682e-01f,	9.8293658286116958e-01f,   9.9627868399859310e-01f,	1.0198955019000133e+00f,   1.0166395501210359e+00f,	1.0220913178757398e+00f,   9.9651666040682441e-01f,	1.0097766178917882e+00f,   1.0215422470827016e+00f,	6.4031953387790963e-01f,   2.5012379477078184e-03f,	6.5339939555769944e-03f,   2.8334080462675826e-03f,	-5.1209675389074505e-11f,  -9.0592291646646381e-03f, 3.3936718323331200e-03f,  -3.0638741121828406e-03f,	2.2203936168286292e-01f,   6.3141140024811970e-01f,	9.7480985576500956e-01f,   9.7209562333590571e-01f,	1.0173770302868150e+00f,   9.9875194322734129e-01f,	9.4701725739602238e-01f,   8.5258623154354796e-01f,	9.4897798581660842e-01f,   9.4751876096521492e-01f,	9.9598944191059791e-01f,   8.6301351503809076e-01f,	8.9150987853523145e-01f,   8.4866492652845082e-01f 	},
		// YELLOW
		{ 5.5740622924920873e-03f,  -4.7982831631446787e-03f, -5.2536564298613798e-03f,  -6.4571480044499710e-03f, -5.9693514658007013e-03f,  -2.1836716037686721e-03f, 1.6781120601055327e-02f,   9.6096355429062641e-02f,	2.1217357081986446e-01f,   3.6169133290685068e-01f,	5.3961011543232529e-01f,   7.4408810492171507e-01f,	9.2209571148394054e-01f,   1.0460304298411225e+00f,	1.0513824989063714e+00f,   1.0511991822135085e+00f,	1.0510530911991052e+00f,   1.0517397230360510e+00f,	1.0516043086790485e+00f,   1.0511944032061460e+00f,	1.0511590325868068e+00f,   1.0516612465483031e+00f,	1.0514038526836869e+00f,   1.0515941029228475e+00f,	1.0511460436960840e+00f,   1.0515123758830476e+00f,	1.0508871369510702e+00f,   1.0508923708102380e+00f,	1.0477492815668303e+00f,   1.0493272144017338e+00f,	1.0435963333422726e+00f,   1.0392280772051465e+00f },
		// RED
		{ 1.6575604867086180e-01f,   1.1846442802747797e-01f,	1.2408293329637447e-01f,   1.1371272058349924e-01f, 7.8992434518899132e-02f,   3.2205603593106549e-02f, -1.0798365407877875e-02f,   1.8051975516730392e-02f, 5.3407196598730527e-03f,   1.3654918729501336e-02f, -5.9564213545642841e-03f,  -1.8444365067353252e-03f, -1.0571884361529504e-02f,  -2.9375521078000011e-03f, -1.0790476271835936e-02f,  -8.0224306697503633e-03f, -2.2669167702495940e-03f,   7.0200240494706634e-03f, -8.1528469000299308e-03f,   6.0772866969252792e-01f, 9.8831560865432400e-01f,   9.9391691044078823e-01f,	1.0039338994753197e+00f,   9.9234499861167125e-01f,	9.9926530858855522e-01f,   1.0084621557617270e+00f,	9.8358296827441216e-01f,   1.0085023660099048e+00f,	9.7451138326568698e-01f,   9.8543269570059944e-01f,	9.3495763980962043e-01f,   9.8713907792319400e-01f },
		// GREEN 
		{ 2.6494153587602255e-03f,  -5.0175013429732242e-03f,	-1.2547236272489583e-02f,  -9.4554964308388671e-03f, -1.2526086181600525e-02f,  -7.9170697760437767e-03f, -7.9955735204175690e-03f,  -9.3559433444469070e-03f, 6.5468611982999303e-02f,   3.9572875517634137e-01f,	7.5244022299886659e-01f,   9.6376478690218559e-01f,	9.9854433855162328e-01f,   9.9992977025287921e-01f,	9.9939086751140449e-01f,   9.9994372267071396e-01f, 9.9939121813418674e-01f,   9.9911237310424483e-01f, 9.6019584878271580e-01f,   6.3186279338432438e-01f, 2.5797401028763473e-01f,   9.4014888527335638e-03f, -3.0798345608649747e-03f,  -4.5230367033685034e-03f, -6.8933410388274038e-03f,  -9.0352195539015398e-03f, -8.5913667165340209e-03f,  -8.3690869120289398e-03f, -7.8685832338754313e-03f,  -8.3657578711085132e-06f, 5.4301225442817177e-03f,  -2.7745589759259194e-03f },
		// BLUE
		{ 9.9209771469720676e-01f,   9.8876426059369127e-01f,	9.9539040744505636e-01f,   9.9529317353008218e-01f,	9.9181447411633950e-01f,   1.0002584039673432e+00f,	9.9968478437342512e-01f,   9.9988120766657174e-01f,	9.8504012146370434e-01f,   7.9029849053031276e-01f,	5.6082198617463974e-01f,   3.3133458513996528e-01f,	1.3692410840839175e-01f,   1.8914906559664151e-02f, -5.1129770932550889e-06f,  -4.2395493167891873e-04f, -4.1934593101534273e-04f,   1.7473028136486615e-03f, 3.7999160177631316e-03f,  -5.5101474906588642e-04f,	-4.3716662898480967e-05f,   7.5874501748732798e-03f, 2.5795650780554021e-02f,   3.8168376532500548e-02f,	4.9489586408030833e-02f,   4.9595992290102905e-02f,	4.9814819505812249e-02f,   3.9840911064978023e-02f,	3.0501024937233868e-02f,   2.1243054765241080e-02f,	6.9596532104356399e-03f,   4.1733649330980525e-03f 	}
	};		
#endif

inline float SampleSpectralBasisFunctions( int spectrum_idx, int function_idx )
{
	return smits_lut[function_idx][spectrum_idx];
}

inline float4 SampleSpectralBasisFunctions4( int4 spectrum_idx, int function_idx )
{
	return (float4)(
		smits_lut[function_idx][spectrum_idx.x],
		smits_lut[function_idx][spectrum_idx.y],
		smits_lut[function_idx][spectrum_idx.z],
		smits_lut[function_idx][spectrum_idx.w]);
}


inline float* RGBtoSpectralf1( float *out, const Vec3 *_rgb, const float *spectrum )
{
	#ifndef SPECTRAL
		(*out) = 0.f;
	#else
		const Vec3 rgb = *_rgb;
	
		const int complement_lut[3] = {SMITS_CYAN_BASE_FUNC,SMITS_MAGENTA_BASE_FUNC,SMITS_YELLOW_BASE_FUNC};
		const int primary_lut[3] = {SMITS_RED_BASE_FUNC,SMITS_GREEN_BASE_FUNC,SMITS_BLUE_BASE_FUNC};
		
		// get piecewise constant spectral interval indices for wavelengths of interest
		int interval_idx = SampleSpectralIntervals1( spectrum, SMITS_MIN_LAMBDA, SMITS_MAX_LAMBDA, SMITS_BINS);

		// get minimum intensity channel
		int min_idx = v3dominis( &rgb );
		
		// set white spectrum
		*out = rgb.v[min_idx] * SampleSpectralBasisFunctions( interval_idx , SMITS_WHITE_BASE_FUNC ) * SMITS_SCALE;
		
		// get remaining indices
		int idx_1 = (min_idx + 1) % 3;
		int idx_2 = (min_idx + 2) % 3;
		
		// swap if idx_1 is not the minimum intensity channel
		if( rgb.v[idx_1] > rgb.v[idx_2] )
		{
			int tmp = idx_1;
			idx_1 = idx_2;
			idx_2 = tmp;
		}
			
		// add complementery color
		*out += (rgb.v[idx_1] - rgb.v[min_idx]) * SampleSpectralBasisFunctions( interval_idx , complement_lut[min_idx] ) * SMITS_SCALE;
		
		// add primary color
		*out += (rgb.v[idx_2] - rgb.v[idx_1]) * SampleSpectralBasisFunctions( interval_idx , primary_lut[idx_2] ) * SMITS_SCALE;
		
		*out = max( *out , 0.f );
	#endif
	return out;
}


inline float4* RGBtoSpectralf4( float4 *out, const Vec3 *_rgb, const float4 *spectrum )
{
	#ifndef SPECTRAL
		(*out).x = _rgb->x;
		(*out).y = _rgb->y;
		(*out).z = _rgb->z;
		(*out).w = 0.f;
	#else
		const Vec3 rgb = *_rgb;
	
		const int complement_lut[3] = {SMITS_CYAN_BASE_FUNC,SMITS_MAGENTA_BASE_FUNC,SMITS_YELLOW_BASE_FUNC};
		const int primary_lut[3] = {SMITS_RED_BASE_FUNC,SMITS_GREEN_BASE_FUNC,SMITS_BLUE_BASE_FUNC};
		
		// get piecewise constant spectral interval indices for wavelengths of interest
		int4 interval_idx = SampleSpectralIntervals4( spectrum, SMITS_MIN_LAMBDA, SMITS_MAX_LAMBDA, SMITS_BINS);

		// get minimum intensity channel
		int min_idx = v3dominis( &rgb );
		
		// set white spectrum
		*out = rgb.v[min_idx] * SampleSpectralBasisFunctions4( interval_idx , SMITS_WHITE_BASE_FUNC ) * SMITS_SCALE;
		
		// get remaining indices
		int idx_1 = (min_idx + 1) % 3;
		int idx_2 = (min_idx + 2) % 3;
		
		// swap if idx_1 is not the minimum intensity channel
		if( rgb.v[idx_1] > rgb.v[idx_2] )
		{
			int tmp = idx_1;
			idx_1 = idx_2;
			idx_2 = tmp;
		}
			
		// add complementery color
		*out += (rgb.v[idx_1] - rgb.v[min_idx]) * SampleSpectralBasisFunctions4( interval_idx , complement_lut[min_idx] ) * SMITS_SCALE;
		
		// add primary color
		*out += (rgb.v[idx_2] - rgb.v[idx_1]) * SampleSpectralBasisFunctions4( interval_idx , primary_lut[idx_2] ) * SMITS_SCALE;
		
		// clamp negatives
		*out = max( *out , 0.f);
	#endif
	return out;
}

#endif
