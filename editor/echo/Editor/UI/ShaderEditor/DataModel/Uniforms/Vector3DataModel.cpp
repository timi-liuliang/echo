#include "Vector3DataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataVector3.h"

namespace DataFlowProgramming
{
    Vector3DataModel::Vector3DataModel()
        : ShaderUniformDataModel()
    {
        m_vector3Editor = (new QT_UI::QVector3Editor(nullptr, "", nullptr));
        m_vector3Editor->setMaximumSize(QSize(m_vector3Editor->sizeHint().width() * 0.4f, m_vector3Editor->sizeHint().height()));
        m_vector3Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector3Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);

		m_uniformConfig = EchoNew(Echo::ShaderUniform);
		m_uniformConfig->onVariableNameChanged.connectClassMethod(this, Echo::createMethodBind(&Vector3DataModel::onVariableNameChanged));

		updateOutputDataVariableName();
    }

    QJsonObject Vector3DataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector3Editor->getValue()).c_str();

        return modelJson;
    }

    void Vector3DataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector3Editor->setValue(v.toString());
        }
    }

	void Vector3DataModel::updateOutputDataVariableName()
	{
		m_outputs[0] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
	}

	void Vector3DataModel::onVariableNameChanged()
	{
		updateOutputDataVariableName();

		onTextEdited();
	}

    void Vector3DataModel::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
    }

	bool Vector3DataModel::generateCode(ShaderCompiler& compiler)
	{
		if (m_uniformConfig->isExport())
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

	bool Vector3DataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		if (m_uniformConfig->isExport())
		{
			Echo::Vector3 number = m_vector3Editor->getValue();

			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(number);
		}

		return false;
	}
}
