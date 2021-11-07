#include "shader_node_vector2.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeVector2::ShaderNodeVector2()
        : ShaderNodeUniform()
    {
		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
    }

	void ShaderNodeVector2::bindMethods()
	{
		CLASS_BIND_METHOD(ShaderNodeVector2, getValue, DEF_METHOD("getValue"));
		CLASS_BIND_METHOD(ShaderNodeVector2, setValue, DEF_METHOD("setValue"));

		CLASS_REGISTER_PROPERTY(ShaderNodeVector2, "Value", Variant::Type::Vector2, "getValue", "setValue");
	}

	void ShaderNodeVector2::setVariableName(const String& variableName)
	{
		m_variableName = variableName;
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));

		Q_EMIT dataUpdated(0);
	}

	void ShaderNodeVector2::setValue(const Vector2& value)
	{
		m_value = value;
	}

	bool ShaderNodeVector2::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_isUniform)
		{
			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(m_value);
		}

		return false;
	}

	bool ShaderNodeVector2::generateCode(Echo::ShaderCompiler& compiler)
	{
        if (m_isUniform)
        {
			compiler.addUniform("vec2", getVariableName().c_str());

			compiler.addCode(Echo::StringUtil::Format("\tfloat %s_Value = fs_ubo.%s;\n", getVariableName().c_str(), getVariableName().c_str()));
        }
        else
        {
			compiler.addCode(Echo::StringUtil::Format("\tvec2 %s_Value = vec2(%f, %f);\n", getVariableName().c_str(), m_value.x, m_value.y));
        }

		return true;
	}
}

#endif