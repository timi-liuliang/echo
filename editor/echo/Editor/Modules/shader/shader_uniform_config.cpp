#include "shader_uniform_config.h"

namespace Echo
{
	ShaderUniform::ShaderUniform()
	{

	}

	ShaderUniform::~ShaderUniform()
	{

	}

	void ShaderUniform::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderUniform, getVariableName, DEF_METHOD("getVariableName"));
		CLASS_BIND_METHOD(ShaderUniform, setVariableName, DEF_METHOD("setVariableName"));

		CLASS_REGISTER_PROPERTY(ShaderUniform, "Name", Variant::Type::String, "getVariableName", "setVariableName");

		CLASS_REGISTER_SIGNAL(ShaderUniform, onVariableNameChanged);
	}

	void ShaderUniform::setVariableName(const String& variableName)
	{ 
		m_variableName = variableName; 

		if (onVariableNameChanged.isHaveConnects())
		{
			onVariableNameChanged();
		}
	}
}