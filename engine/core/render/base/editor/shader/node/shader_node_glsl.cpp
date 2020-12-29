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

	void ShaderNodeGLSL::setInputs(const String& inputs)
	{ 
		m_inputs = inputs;
	}

	ShaderNode::DataTypes ShaderNodeGLSL::getInputDataTypes()
	{
		DataTypes result;
		StringArray inputs = StringUtil::Split(m_inputs, ",");
		for (const String& input : inputs)
		{
			StringArray inputInfo = StringUtil::Split(input, " ");
			if (inputInfo.size() == 2)
			{
				result.push_back({inputInfo[0], inputInfo[1]});
			}
		}

		return result;
	}

	bool ShaderNodeGLSL::generateCode(ShaderCompiler& compiler)
	{
		String functionCode = StringUtil::Format("%s custom_fun_%d( %s)\n{\n%s\n}", m_returnType.getValue().c_str(), m_id, m_inputs.c_str(), m_code.c_str());
		compiler.addFunction(functionCode);

		return true;
	}

#endif
}

