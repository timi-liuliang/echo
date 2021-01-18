#include "Vector4DataModel .h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>

using namespace Echo;

namespace DataFlowProgramming
{
    Vector4DataModel::Vector4DataModel()
        : ShaderUniformDataModel()
    {
        m_vector4Editor = (new QT_UI::QVector4Editor(nullptr, "", nullptr));
        m_vector4Editor->setMaximumSize(QSize(m_vector4Editor->sizeHint().width() * 0.4f, m_vector4Editor->sizeHint().height()));
        m_vector4Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector4Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);

		m_uniformConfig = EchoNew(Echo::ShaderNodeUniform);
		m_uniformConfig->onVariableNameChanged.connectClassMethod(this, Echo::createMethodBind(&Vector4DataModel::onVariableNameChanged));

		updateOutputDataVariableName();
    }


    QJsonObject Vector4DataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector4Editor->getValue()).c_str();

        return modelJson;
    }

    void Vector4DataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector4Editor->setValue(v.toString());
        }
    }

	void Vector4DataModel::updateOutputDataVariableName()
	{
		m_outputs[0] = std::make_shared<DataVector4>(this, "vec4");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
	}

	void Vector4DataModel::onVariableNameChanged()
	{
		updateOutputDataVariableName();

		onTextEdited();
	}

    void Vector4DataModel::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
    }

	bool Vector4DataModel::generateCode(Echo::ShaderCompiler& compiler)
	{
        if (m_uniformConfig->isExport())
        {
			compiler.addUniform("vec4", getVariableName().c_str());

			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = fs_ubo.%s;\n", getVariableName().c_str(), getVariableName().c_str()));
        }
        else
        {
			Echo::Vector4 number = m_vector4Editor->getValue();
			compiler.addCode(Echo::StringUtil::Format("\tvec4 %s_Value = vec4(%f, %f, %f, %f);\n", getVariableName().c_str(), number.x, number.y, number.z, number.w));
        }

		return true;
	}

	bool Vector4DataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
        if (m_uniformConfig->isExport())
        {
			Echo::Vector2 number = m_vector4Editor->getValue();

			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(number);
        }

		return false;
	}
}
