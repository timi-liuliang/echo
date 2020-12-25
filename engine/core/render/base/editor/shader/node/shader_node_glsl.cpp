#include "shader_node_glsl.h"
#include "ui/parameter_list_editor_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeGLSL::ShaderNodeGLSL()
		: ShaderNode()
	{
		m_caption = "GLSL";
	}

	ShaderNodeGLSL::~ShaderNodeGLSL()
	{
	}

	void ShaderNodeGLSL::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeGLSL, getInputs, DEF_METHOD("getInputs"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setInputs, DEF_METHOD("setInputs"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, getCode, DEF_METHOD("getCode"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setCode, DEF_METHOD("setCode"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, getReturnType, DEF_METHOD("getReturnType"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setReturnType, DEF_METHOD("setReturnType"));

		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "Inputs", Variant::Type::String, "getInputs", "setInputs");
		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "ReturnType", Variant::Type::StringOption, "getReturnType", "setReturnType");
		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "Code", Variant::Type::String, "getCode", "setCode");
		CLASS_REGISTER_PROPERTY_HINT(ShaderNodeGLSL, "Code", PropertyHintType::Language, "glsl");

		REGISTER_PROPERTY_EDITOR(ShaderNodeGLSL, "Inputs", ParamterListEditorGLSL);
	}

#endif
}

