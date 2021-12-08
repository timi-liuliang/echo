#include "shader_node_color.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeColor::ShaderNodeColor()
        : ShaderNodeUniform()
    {
        m_outputs.resize(2);
		m_outputs[0] = std::make_shared<DataVector3>(this, "rgb");
		m_outputs[1] = std::make_shared<DataFloat>(this, "a");

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value.rgb", getVariableName().c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s_Value.a", getVariableName().c_str()));
    }

    void ShaderNodeColor::bindMethods()
    {
		CLASS_BIND_METHOD(ShaderNodeColor, getColor);
		CLASS_BIND_METHOD(ShaderNodeColor, setColor);

		CLASS_REGISTER_PROPERTY(ShaderNodeColor, "Color", Variant::Type::Color, getColor, setColor);
    }

    void ShaderNodeColor::setColor(const Color& color)
    {
        m_color = color;

		Q_EMIT dataUpdated(0);
		Q_EMIT dataUpdated(1);
    }

    const Color& ShaderNodeColor::getColor()
    {
        return m_color;
    }

	void ShaderNodeColor::setVariableName(const String& variableName)
    { 
        m_variableName = variableName; 

		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value.rgb", getVariableName().c_str()));
		m_outputs[1]->setVariableName(Echo::StringUtil::Format("%s_Value.a", getVariableName().c_str()));
    }

	bool ShaderNodeColor::generateCode(Echo::ShaderCompiler& compiler)
	{
		if (m_isUniform)
		{
			compiler.addUniform("vec4", getVariableName().c_str());

            compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(SRgbToLinear(fs_ubo.%s.rgb), fs_ubo.%s.a);\n", getVariableName().c_str(), getVariableName().c_str(), getVariableName().c_str()));
		}
		else
		{
			Echo::Color color = m_color;
            color.toLinear();

			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), color.r, color.g, color.b, color.a));
		}

		return true;
	}

    bool ShaderNodeColor::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
    {
        if (m_isUniform)
        {
            uniformNames.emplace_back("Uniforms." + getVariableName());
            uniformValues.emplace_back(m_color);

            return true;
        }

        return false;
    }
}

#endif