#include "shader_node_vector3.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	ShaderNodeVector3::ShaderNodeVector3()
        : ShaderNodeUniform()
    {
        m_vector3Editor = (new QT_UI::QVector3Editor(nullptr, "", nullptr));
        m_vector3Editor->setMaximumSize(QSize(m_vector3Editor->sizeHint().width() * 0.4f, m_vector3Editor->sizeHint().height()));
        m_vector3Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector3Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);

		updateOutputDataVariableName();
    }

    QJsonObject ShaderNodeVector3::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector3Editor->getValue()).c_str();

        return modelJson;
    }

    void ShaderNodeVector3::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector3Editor->setValue(v.toString());
        }
    }

	void ShaderNodeVector3::updateOutputDataVariableName()
	{
		m_outputs[0] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
	}

	void ShaderNodeVector3::onVariableNameChanged()
	{
		updateOutputDataVariableName();

		onTextEdited();
	}

    void ShaderNodeVector3::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
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
			Echo::Vector3 number = m_vector3Editor->getValue();
			compiler.addCode(Echo::StringUtil::Format("\tvec3 %s_Value = vec3(%f, %f, %f);\n", getVariableName().c_str(), number.x, number.y, number.z));
		}

		return true;
	}

	bool ShaderNodeVector3::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_isUniform)
		{
			Echo::Vector3 number = m_vector3Editor->getValue();

			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(number);
		}

		return false;
	}
}

#endif