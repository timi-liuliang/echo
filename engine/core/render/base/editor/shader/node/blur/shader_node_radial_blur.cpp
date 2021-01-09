#include "shader_node_radial_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* radialBlur = R"(vec3 RadialBlur(sampler2D tex, vec2 uv,vec2 center, float quality, float strength)
{
	// https://gaming.stackexchange.com/questions/306721/what-is-radial-blur
	vec2 dir = (center - uv) / quality;
	vec4 origin = texture(tex, uv);
	vec4 color = origin;
	float count = 1.0;
	
	for (float i = 1.0; i <= quality; i += 1.0)
	{
		color += texture(tex, uv + dir * i);
		count += 1.0;
	}

	color /= count;

	return mix(origin.xyz, color.xyz, strength);
})";

namespace Echo
{
	ShaderNodeRadialBlur::ShaderNodeRadialBlur()
		: ShaderNodeGLSL()
	{
		setCode(radialBlur);
	}

	ShaderNodeRadialBlur::~ShaderNodeRadialBlur()
	{
	}

	void ShaderNodeRadialBlur::bindMethods()
	{
	}
}

#endif
