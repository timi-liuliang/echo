#include "shader_node_zoom_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* zoomBlur = R"(vec3 ZoomBlur(sampler2D tex, vec2 uv, vec2 center, float radius, float samples, float strength)
{
	// https://gaming.stackexchange.com/questions/306721/what-is-radial-blur
	vec2 dir = center - uv;
	float len = length(dir);
	float scale = clamp(len, 0.0, radius) / len;
	vec2 step = dir * scale / samples;
	vec4 origin = texture(tex, uv);
	vec4 color = origin;
	float count = 1.0;
	
	for (float i = 1.0; i <= samples; i += 1.0)
	{
		color += texture(tex, uv + step * i);
		count += 1.0;
	}

	color /= count;

	return mix(origin.xyz, color.xyz, strength);
})";

namespace Echo
{
	ShaderNodeZoomBlur::ShaderNodeZoomBlur()
		: ShaderNodeGLSL()
	{
		setCode(zoomBlur);
	}

	ShaderNodeZoomBlur::~ShaderNodeZoomBlur()
	{
	}

	void ShaderNodeZoomBlur::bindMethods()
	{
	}
}

#endif
