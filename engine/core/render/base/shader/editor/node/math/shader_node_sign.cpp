#include "shader_node_sign.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(float Sign(float number)
{
	// https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/sign.xhtml
	// sign returns -1.0 if x is less than 0.0, 0.0 if x is equal to 0.0, and +1.0 if x is greater than 0.0.

	return sign(number);
})";

namespace Echo
{
	ShaderNodeSign::ShaderNodeSign()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeSign::~ShaderNodeSign()
	{
	}

	void ShaderNodeSign::bindMethods()
	{
	}
}

#endif
