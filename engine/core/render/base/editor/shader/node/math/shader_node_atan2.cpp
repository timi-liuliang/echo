#include "shader_node_atan2.h"

#ifdef ECHO_EDITOR_MODE

static const char* code = R"(float ATan2(vec2 dir)
{
	float angle = asin(dir.x) > 0 ? acos(dir.y) : -acos(dir.y);
	return angle;
})";

namespace Echo
{
	ShaderNodeATan2::ShaderNodeATan2()
		: ShaderNodeGLSL()
	{
		setCode(code);
	}

	ShaderNodeATan2::~ShaderNodeATan2()
	{
	}

	void ShaderNodeATan2::bindMethods()
	{
	}
}

#endif
