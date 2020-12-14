#include "shader_node_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeGLSL::ShaderNodeGLSL()
		: ShaderNode()
	{
	}

	ShaderNodeGLSL::~ShaderNodeGLSL()
	{
	}

	void ShaderNodeGLSL::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeGLSL, getCode, DEF_METHOD("getCode"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setCode, DEF_METHOD("setCode"));

		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "Code", Variant::Type::ResourcePath, "getCode", "setCode");
		CLASS_REGISTER_PROPERTY_HINT(ShaderNodeGLSL, "Code", PropertyHintType::Language, "glsl");
	}

#endif
}

