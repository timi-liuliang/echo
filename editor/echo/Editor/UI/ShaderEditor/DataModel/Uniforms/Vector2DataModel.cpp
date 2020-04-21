#include "Vector2DataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataVector3.h"
#include "DataVector2.h"

namespace DataFlowProgramming
{
    Vector2DataModel::Vector2DataModel()
        : ShaderUniformDataModel()
    {
        m_uniformConfig = EchoNew(Echo::ShaderUniform);

        m_vector2Editor = (new QT_UI::QVector2Editor(nullptr, "", nullptr));
        m_vector2Editor->setMaximumSize(QSize(m_vector2Editor->sizeHint().width() * 0.4f, m_vector2Editor->sizeHint().height()));
        m_vector2Editor->setValue(Echo::StringUtil::ToString(Echo::Vector3::ONE).c_str());

        QObject::connect(m_vector2Editor, SIGNAL(Signal_ValueChanged()), this, SLOT(onTextEdited()));

		m_outputs.resize(1);
		m_outputs[0] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[0]->setVariableName(Echo::StringUtil::Format("%s", getVariableName().c_str()));
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

    unsigned int Vector2DataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
      case PortType::In:  return m_inputs.size();
      case PortType::Out: return m_outputs.size();
      default:            return 0;
      }
    }

    void Vector2DataModel::onTextEdited()
    {
        Q_EMIT dataUpdated(0);
    }

    NodeDataType Vector2DataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        return portType == PortType::Out ? m_outputs[portIndex]->type() : NodeDataType{ "invalid", "invalid" };
    }

    std::shared_ptr<NodeData> Vector2DataModel::outData(PortIndex portIndex)
    {
        return m_outputs[portIndex];
    }

	bool Vector2DataModel::generateCode(ShaderCompiler& compiler)
	{
		Echo::Vector2 number = m_vector2Editor->getValue();
		compiler.addCode(Echo::StringUtil::Format("\tvec2 %s = vec2(%f, %f);\n", getVariableName().c_str(), number.x, number.y));

		return true;
	}

	bool Vector2DataModel::getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues)
	{
		return false;
	}
}
