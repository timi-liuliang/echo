#include "shader_node_gray_scale.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(vec3 GrayScale(vec3 rgb)
{
	float gray = dot(rgb, vec3(0.3, 0.59, 0.11));
	return vec3(gray, gray, gray);
})";

namespace Echo
{
	ShaderNodeGrayScale::ShaderNodeGrayScale()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeGrayScale::~ShaderNodeGrayScale()
	{
	}

	void ShaderNodeGrayScale::bindMethods()
	{
	}
}

#endif
