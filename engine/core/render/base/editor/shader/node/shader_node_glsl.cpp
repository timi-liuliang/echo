#include "shader_node_glsl.h"
#include "ui/parameter_list_editor_glsl.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeGLSL::ShaderNodeGLSL()
		: ShaderNode()
	{
		m_caption = "GLSL";

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataFloat>(this, "float");
		m_outputs[0]->setVariableName(getDefaultVariableName());
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

		if (m_inputDataTypes != getInputDataTypes(inputs))
		{
			m_inputDataTypes = getInputDataTypes(inputs);
			m_inputs.resize(m_inputDataTypes.size());

			Q_EMIT portUpdated();
		}
	}

	QtNodes::NodeDataTypes ShaderNodeGLSL::getInputDataTypes(const String& inputs)
	{
		QtNodes::NodeDataTypes result;
		StringArray inputArray = StringUtil::Split(inputs, ",");
		for (const String& input : inputArray)
		{
			StringArray inputInfo = StringUtil::Split(input, " ");
			if (inputInfo.size() == 2)
			{
				result.push_back({inputInfo[0], inputInfo[1]});
			}
		}

		return result;
	}

	void ShaderNodeGLSL::setReturnType(const StringOption& type)
	{ 
		if (m_returnType.setValue(type.getValue()))
		{
			QtNodes::NodeDataModel* model = this;

			m_outputs.resize(1);
			m_outputs[0]->setVariableName(getDefaultVariableName());

			if(m_returnType.getValue()=="")				m_outputs[0] = std::make_shared<DataFloat>(model, "float");
			else if (m_returnType.getValue() == "vec2")	m_outputs[0] = std::make_shared<DataVector2>(model, "vec2");
			else if (m_returnType.getValue() == "vec3") m_outputs[0] = std::make_shared<DataVector3>(model, "vec3");
			else if (m_returnType.getValue() == "vec4") m_outputs[0] = std::make_shared<DataVector4>(model, "vec4");
			else										m_outputs[0] = std::make_shared<DataInvalid>(model);

			Q_EMIT portUpdated();
		}
	}

	bool ShaderNodeGLSL::generateCode(ShaderCompiler& compiler)
	{
		String functionCode = StringUtil::Format("%s custom_fun_%d( %s)\n{\n%s\n}", m_returnType.getValue().c_str(), m_id, m_inputs.c_str(), m_code.c_str());
		compiler.addFunction(functionCode);

		return true;
	}

#endif
}

