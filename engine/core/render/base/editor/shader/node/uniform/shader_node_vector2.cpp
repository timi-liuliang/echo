#include "Vector2DataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>

using namespace Echo;

namespace DataFlowProgramming
{
    Vector2DataModel::Vector2DataModel()
        : ShaderUniformDataModel()
    {
        m_vector2Editor = (new QT_UI::QVector2Editor(nullptr, "", nullptr));
        m_vector2Editor->setMaximumSize(QSize(m_vector2Editor->sizeHint().width() * 0.4f, m_vector2Editor->sizeHint().height()));
        m_vector2Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector2Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);

		m_uniformConfig = EchoNew(Echo::ShaderNodeUniform);
		m_uniformConfig->onVariableNameChanged.connectClassMethod(this, Echo::createMethodBind(&Vector2DataModel::onVariableNameChanged));

        updateOutputDataVariableName();
    }


    QJsonObject Vector2DataModel::save() const
    {
        QJsonObject modelJson = NodeDataModel::save();

		modelJson["number"] = Echo::StringUtil::ToString(m_vector2Editor->getValue()).c_str();

        return modelJson;
    }

    void Vector2DataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["number"];
        if (!v.isUndefined())
        {
            QString strNum = v.toString();
            m_vector2Editor->setValue(v.toString());
        }
    }

	void Vector2DataModel::updateOutputDataVariableName()
	{
		m_outputs[0] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s_Value", getVariableName().c_str()));
	}

	void Vector2DataModel::onVariableNameChanged()
	{
		updateOutputDataVariableName();

		onTextEdited();
	}

    void Vector2DataModel::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
    }

	bool Vector2DataModel::generateCode(Echo::ShaderCompiler& compiler)
	{
        if (m_uniformConfig->isExport())
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

	bool Vector2DataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
        if (m_uniformConfig->isExport())
        {
            Echo::Vector2 number = m_vector2Editor->getValue();

			uniformNames.emplace_back("Uniforms." + getVariableName());
			uniformValues.emplace_back(number);
        }

		return false;
	}
}
