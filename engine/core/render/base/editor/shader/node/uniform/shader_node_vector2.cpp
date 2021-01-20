#include "shader_node_vector2.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeVector2::ShaderNodeVector2()
        : ShaderNodeUniform()
    {
        m_vector2Editor = (new QT_UI::QVector2Editor(nullptr, "", nullptr));
        m_vector2Editor->setMaximumSize(QSize(m_vector2Editor->sizeHint().width() * 0.4f, m_vector2Editor->sizeHint().height()));
        m_vector2Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector2Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);

        updateOutputDataVariableName();
    }


    QJsonObject ShaderNodeVector2::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector2Editor->getValue()).c_str();

        return modelJson;
    }

    void ShaderNodeVector2::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector2Editor->setValue(v.toString());
        }
    }

	void ShaderNodeVector2::updateOutputDataVariableName()
	{
		m_outputs[0] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
	}

	void ShaderNodeVector2::onVariableNameChanged()
	{
		updateOutputDataVariableName();

		onTextEdited();
	}

    void ShaderNodeVector2::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
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
			Echo::Vector2 number = m_vector2Editor->getValue();
			compiler.addCode(Echo::StringUtil::Format("\tvec2 %s_Value = vec2(%f, %f);\n", getVariableName().c_str(), number.x, number.y));
        }

		return true;
	}

	bool ShaderNodeVector2::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
        if (m_isUniform)
        {
            Echo::Vector2 number = m_vector2Editor->getValue();

			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(number);
        }

		return false;
	}
}

#endif