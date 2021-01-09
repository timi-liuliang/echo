#include "shader_node_radial_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* radialBlur = R"(vec3 RadialBlur(sampler2D tex, vec2 uv,vec2 center, float quality)
{
	vec2 dir = (center - uv) / quality;
	vec4 color = vec4( 0.0);

	float count = 0.0;
	for (float i = 0.0; i <= quality; i += 1.0)
	{
		color += texture(tex, uv + dir * i);
		count += 1.0;
	}

	color /= count;

	return color.xyz;
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
