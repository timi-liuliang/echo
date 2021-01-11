#include "shader_node_spin_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* radialBlur = R"(vec3 SpinBlur(sampler2D tex, vec2 uv, vec2 center, float speed, float samples, float strength)
{
	float len = length(uv - center);	
	vec2 dir = normalize(uv - center);
	float angle = atan(dir.x, dir.y);
	float step = radians(speed) / samples;
	vec4 origin = texture(tex, uv);
	vec4 color = origin;
	float count = 1.0;
	
	for (float i = 1.0; i <= samples; i += 1.0)
	{
		float d = angle + step * i;
		color += texture(tex, center + len * vec2(cos(d), sin(d)));
		count += 1.0;
	}

	color /= count;

	return mix(origin.xyz, color.xyz, strength);
})";

namespace Echo
{
	ShaderNodeSpinBlur::ShaderNodeSpinBlur()
		: ShaderNodeGLSL()
	{
		setCode(radialBlur);
	}

	ShaderNodeSpinBlur::~ShaderNodeSpinBlur()
	{
	}

	void ShaderNodeSpinBlur::bindMethods()
	{
	}
}

#endif
