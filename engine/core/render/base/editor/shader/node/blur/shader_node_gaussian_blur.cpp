#include "shader_node_gaussian_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* gaussianBlur =R"(vec3 GaussianBlur(sampler2D tex,vec2 uv,float dirs)
{
	// https://www.shadertoy.com/view/Xltfzj
	float pi = 6.28318530718;
	float directions = dirs;	// blur directions (default 16.0 - more is better but slower)
	float quality = 3.0;		// blur quality (default 4.0 - more is better but slower)
	float radius = 8.0 / 512.0;	// blur radius

	vec4 color = texture(tex, uv);

	for (float d = 0.0; d < pi; d += pi / directions)
	{
		for (float i = 1.0 / quality; i <= 1.0; i += 1.0 / quality)
		{
			color += texture(tex, uv + vec2(cos(d), sin(d)) * radius * i);
		}
	}

	color /= quality * directions - directions + 1.0;

	return color.xyz;
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
