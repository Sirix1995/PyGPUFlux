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

#ifndef _SWITCH_SHADER_H
#define _SWITCH_SHADER_H

#include "shader/shader.h"

typedef struct
{
	Shader base;
	
	ShaderHandle frontShader, backShader;
} SwitchShader;

	
inline const __global Shader* evalSwitchShader( ShaderHandle shaderHandle, const __global char *shaders, const Vec3 *in, const Vec3 *norm )
{
	const __global Shader* shader = (const __global Shader*)&shaders[shaderHandle];
	
	// dereference switch shader
	if( shader->type == SHADER_SWITCH )
	{
		const __global SwitchShader *switchshader = (const __global SwitchShader *)shader;
		
		// switch on incomming side
		if( v3dot( in, norm ) > 0.f )
			shaderHandle = switchshader->frontShader;
		else
			shaderHandle = switchshader->backShader;

		// set side-specific shader
		shader = (const __global Shader*)&shaders[shaderHandle];
	}
	
	return shader;
}

#endif

