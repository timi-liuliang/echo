#include "shader_node_vector4.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeVector4::ShaderNodeVector4()
        : ShaderNodeUniform()
    {
    }

	void ShaderNodeVector4::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeVector4, getValue, DEF_METHOD("getValue"));
		CLASS_BIND_METHOD(ShaderNodeVector4, setValue, DEF_METHOD("setValue"));

		CLASS_REGISTER_PROPERTY(ShaderNodeVector4, "Value", Variant::Type::Vector4, "getValue", "setValue");
	}

	void ShaderNodeVector4::setVariableName(const String& variableName)
	{
		m_variableName = variableName;

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataVector4>(this, "vec4");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));

		Q_EMIT dataUpdated(0);
	}

	void ShaderNodeVector4::setValue(const Vector4& value)
	{
		m_value = value;
	}

	bool ShaderNodeVector4::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_isUniform)
		{
			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(m_value);
		}

		return false;
	}

	bool ShaderNodeVector4::generateCode(Echo::ShaderCompiler& compiler)
	{
        if (m_isUniform)
        {
			compiler.addUniform("vec4", getVariableName().c_str());

			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = fs_ubo.%s;\n", getVariableName().c_str(), getVariableName().c_str()));
        }
        else
        {
			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), m_value.x, m_value.y, m_value.z, m_value.w));
        }

		return true;
	}
}

#endif