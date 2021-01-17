#include "shader_node_linear_to_srgb.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(vec3 LinearToSRgb(vec3 linear)
{
	return pow(linear, vec3(1.0/2.2));
})";

namespace Echo
{
	ShaderNodeLinearToSRgb::ShaderNodeLinearToSRgb()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeLinearToSRgb::~ShaderNodeLinearToSRgb()
	{
	}

	void ShaderNodeLinearToSRgb::bindMethods()
	{
	}
}

#endif
