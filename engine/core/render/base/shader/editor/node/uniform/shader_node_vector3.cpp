#include "shader_node_vector3.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderNodeVector3::ShaderNodeVector3()
        : ShaderNodeUniform()
    {
		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
    }

	void ShaderNodeVector3::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeVector3, getValue, DEF_METHOD("getValue"));
		CLASS_BIND_METHOD(ShaderNodeVector3, setValue, DEF_METHOD("setValue"));

		CLASS_REGISTER_PROPERTY(ShaderNodeVector3, "Value", Variant::Type::Vector3, "getValue", "setValue");
	}

	void ShaderNodeVector3::setVariableName(const String& variableName)
	{
		m_variableName = variableName;
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));

		Q_EMIT dataUpdated(0);
	}

	void ShaderNodeVector3::setValue(const Vector3& value)
	{
		m_value = value;
	}

	bool ShaderNodeVector3::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_isUniform)
		{
			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(m_value);
		}

		return false;
	}

	bool ShaderNodeVector3::generateCode(Echo::ShaderCompiler& compiler)
	{
		if (m_isUniform)
		{
			compiler.addUniform("vec3", getVariableName().c_str());

			compiler.addCode(Echo::StringUtil::Format("\tvec3 %s_Value = fs_ubo.%s;\n", getVariableName().c_str(), getVariableName().c_str()));
		}
		else
		{
			compiler.addCode(Echo::StringUtil::Format("\tvec3 %s_Value = vec3(%f, %f, %f);\n", getVariableName().c_str(), m_value.x, m_value.y, m_value.z));
		}

		return true;
	}
}

#endif