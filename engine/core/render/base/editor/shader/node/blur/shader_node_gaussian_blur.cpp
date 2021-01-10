#include "shader_node_gaussian_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* gaussianBlur =R"(vec3 GaussianBlur(sampler2D tex,vec2 uv, float radius, float dirs, float samples, float strength)
{
	// https://www.shadertoy.com/view/Xltfzj
	float pi = 6.28318530718;
	float directions = dirs;	// blur directions (default 16.0 - more is better but slower)
	float step = 1.0 / samples * radius;
	vec4 origin = texture(tex, uv);
	vec4 color = origin;
	float count = 1.0;
	
	for (float d = 0.0; d < pi; d += pi / directions)
	{
		for (float i = 1.0; i <= samples; i += 1.0)
		{
			color += texture(tex, uv + vec2(cos(d), sin(d)) * step * i);
			count += 1.0;
		}
	}

	color /= count;

	return mix(origin.xyz, color.xyz, strength);
})";

namespace Echo
{
	ShaderNodeGaussianBlur::ShaderNodeGaussianBlur()
		: ShaderNodeGLSL()
	{
		setCode(gaussianBlur);
	}

	ShaderNodeGaussianBlur::~ShaderNodeGaussianBlur()
	{
	}

	void ShaderNodeGaussianBlur::bindMethods()
	{
	}
}

#endif
