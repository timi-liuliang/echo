#include "shader_node_flow_blur.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(vec3 FlowBlur(sampler2D tex, sampler2D flow, vec2 uv, float length, float samples, float weight, float strength)
{
	float step = length / samples / 100.0;
	vec4 origin = texture(tex, uv);
	vec4 color = origin;
	float count = 1.0;
	
	for (float i = 1.0; i <= samples; i += 1.0)
	{
		vec2 dir = texture(flow, uv).xy * 2.0 - 1.0;
		uv += dir * step;
		
		float weightPow = pow(clamp(1.0 - i / samples, 0.0, 1.0), weight);
		color += texture(tex, uv) * weightPow;
		count += weightPow;
	}

	color /= count;

	return mix(origin.xyz, color.xyz, strength);
})";

namespace Echo
{
	ShaderNodeFlowBlur::ShaderNodeFlowBlur()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeFlowBlur::~ShaderNodeFlowBlur()
	{
	}

	void ShaderNodeFlowBlur::bindMethods()
	{
	}
}

#endif
