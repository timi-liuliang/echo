#include "shader_node_uniform.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	ShaderNodeUniform::ShaderNodeUniform()
	{

	}

	ShaderNodeUniform::~ShaderNodeUniform()
	{

	}

	void ShaderNodeUniform::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeUniform, isExport, DEF_METHOD("isExport"));
		CLASS_BIND_METHOD(ShaderNodeUniform, setExport, DEF_METHOD("setExport"));
		CLASS_BIND_METHOD(ShaderNodeUniform, getVariableName, DEF_METHOD("getVariableName"));
		CLASS_BIND_METHOD(ShaderNodeUniform, setVariableName, DEF_METHOD("setVariableName"));

		CLASS_REGISTER_PROPERTY(ShaderNodeUniform, "Export", Variant::Type::Bool, "isExport", "setExport");
		CLASS_REGISTER_PROPERTY(ShaderNodeUniform, "Name", Variant::Type::String, "getVariableName", "setVariableName");

		CLASS_REGISTER_SIGNAL(ShaderNodeUniform, onVariableNameChanged);
	}

	void ShaderNodeUniform::setVariableName(const String& variableName)
	{ 
		m_variableName = variableName; 

		if (onVariableNameChanged.isHaveConnects())
		{
			onVariableNameChanged();
		}
	}
#endif
}