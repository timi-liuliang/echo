#include "shader_node_glsl.h"
#include "ui/parameter_list_editor_glsl.h"
#include "engine/core/util/hash_generator.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeGLSL::ShaderNodeGLSL()
		: ShaderNode()
	{
		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataFloat>(this, "float");
		m_outputs[0]->setVariableName(getVariableName());
	}

	ShaderNodeGLSL::~ShaderNodeGLSL()
	{
	}

	void ShaderNodeGLSL::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeGLSL, getFunctionName, DEF_METHOD("getFunctionName"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setFunctionName, DEF_METHOD("setFunctionName"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, getParms, DEF_METHOD("getParms"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setParms, DEF_METHOD("setParms"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, getCode, DEF_METHOD("getCode"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setCode, DEF_METHOD("setCode"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, getReturnType, DEF_METHOD("getReturnType"));
		CLASS_BIND_METHOD(ShaderNodeGLSL, setReturnType, DEF_METHOD("setReturnType"));

		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "FunctionName", Variant::Type::String, "getFunctionName", "setFunctionName");
		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "Parameters", Variant::Type::String, "getParms", "setParms");
		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "ReturnType", Variant::Type::StringOption, "getReturnType", "setReturnType");
		CLASS_REGISTER_PROPERTY(ShaderNodeGLSL, "Code", Variant::Type::String, "getCode", "setCode");
		CLASS_REGISTER_PROPERTY_HINT(ShaderNodeGLSL, "Code", PropertyHintType::Language, "glsl");

		REGISTER_PROPERTY_EDITOR(ShaderNodeGLSL, "Parameters", ParamterListEditorGLSL);
	}

	String ShaderNodeGLSL::getFunctionName() const
	{
		return m_funName.empty() ? StringUtil::Format("custom_fun_%d", m_id) : m_funName;;
	}

	void  ShaderNodeGLSL::setFunctionName(const String& funName)
	{
		m_funName = funName;

		Q_EMIT captionUpdated();
	}

	bool ShaderNodeGLSL::isParamsValid(const String& params)
	{
		StringArray paramTypes = { "float", "vec2", "vec3", "vec4", "sampler2D" };

		StringArray inputArray = StringUtil::Split(params, ",");
		for (const String& input : inputArray)
		{
			StringArray inputInfo = StringUtil::Split(input, " ");
			if (inputInfo.size() != 2)
				return false;

			if (std::find(paramTypes.begin(), paramTypes.end(), inputInfo[0]) == paramTypes.end())
				return false;
		}

		return true;
	}

	ui32 ShaderNodeGLSL::getHash()
	{
		String uniqueStr = m_returnType.getValue() + m_parameters + m_body;
		return BKDRHash(uniqueStr.c_str());
	}

	void ShaderNodeGLSL::setParms(const String& params)
	{ 
		if (isParamsValid(params))
		{
			if (m_parameters != params)
			{
				m_parameters = params;

				m_inputDataTypes = getInputDataTypes(params);
				m_inputs.resize(m_inputDataTypes.size());

				Q_EMIT portUpdated();
			}
		}
	}

	String ShaderNodeGLSL::getCode() const
	{
		String funName = getFunctionName();
		String functionCode = StringUtil::Format("%s %s(%s)\n%s", m_returnType.getValue().c_str(), funName.c_str(), m_parameters.c_str(), m_body.c_str());

		return functionCode;
	}

	void ShaderNodeGLSL::setCode(const String& code)
	{
		StringArray words = StringUtil::Split(code, " ");
		String returnType = words[0];
		setReturnType(returnType);

		String funName = StringUtil::Substr(code, "(", true);
		words = StringUtil::Split(funName, " ");
		if(words.size()==2)
			setFunctionName(words[1]);

		String params = StringUtil::Substr(code, "{", true);
		params = StringUtil::Substr(params, "(", false);
		params = StringUtil::Substr(params, ")", true);
		setParms(params);

		size_t bodyFirstPos = code.find_first_of("{");
		size_t bodyLastPos = code.find_last_of("}");
		if (bodyFirstPos != String::npos && bodyLastPos != String::npos)
		{
			String body = code.substr(bodyFirstPos, bodyLastPos);
			m_body = body;
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
			m_outputs[0]->setVariableName(getVariableName());

			if(m_returnType.getValue()=="float")		m_outputs[0] = std::make_shared<DataFloat>(model, "float");
			else if (m_returnType.getValue() == "vec2")	m_outputs[0] = std::make_shared<DataVector2>(model, "vec2");
			else if (m_returnType.getValue() == "vec3") m_outputs[0] = std::make_shared<DataVector3>(model, "vec3");
			else if (m_returnType.getValue() == "vec4") m_outputs[0] = std::make_shared<DataVector4>(model, "vec4");
			else										m_outputs[0] = std::make_shared<DataInvalid>(model);

			m_outputs[0]->setVariableName(getVariableName());

			Q_EMIT portUpdated();
		}
	}

	bool ShaderNodeGLSL::checkValidation()
	{
		if (!ShaderNode::checkValidation())
			return false;

		for (size_t i = 0; i < m_inputs.size(); i++)
		{
			if (!m_inputs[i])
			{
				m_modelValidationState = QtNodes::NodeValidationState::Error;
				m_modelValidationError = Echo::StringUtil::Format("Input [%d] can't be null", i).c_str();

				return false;
			}
		}

		return true;
	}

	bool ShaderNodeGLSL::generateCode(ShaderCompiler& compiler)
	{
		if (getOutputConnectionCount(0))
		{
			String funName = getFunctionName();
			String functionCode = getCode();
			compiler.addFunction(getHash(), funName, functionCode);

			String params;
			i32 finalIdx = m_inputs.size() - 1;
			for (i32 i = 0; i <= finalIdx; i++)
			{
				if (m_inputs[i])
				{
					params += " " + m_inputs[i]->getVariableName() + (i == finalIdx ? "" : ",");
				}
			}

			String code = StringUtil::Format("\t%s %s = %s(%s);\n", m_returnType.getValue().c_str(), getVariableName().c_str(), funName.c_str(), params.c_str());
			compiler.addCode(code);

			return true;
		}

		return false;
	}

#endif
}

