#include "shader_node_srgb_to_linear.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(vec3 SRgbToLinear(vec3 sRgb)
{
	return pow(sRgb, vec3(2.2));
})";

namespace Echo
{
	ShaderNodeSRgbToLinear::ShaderNodeSRgbToLinear()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeSRgbToLinear::~ShaderNodeSRgbToLinear()
	{
	}

	void ShaderNodeSRgbToLinear::bindMethods()
	{
	}
}

#endif
