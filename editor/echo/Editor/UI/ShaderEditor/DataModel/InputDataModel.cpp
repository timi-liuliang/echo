#include "InputDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"

namespace DataFlowProgramming
{
    InputDataModel::InputDataModel()
      : m_comboBox(new QComboBox())
    {
        m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

        m_outputs.resize(7);

        // position
        m_comboBox->addItem("position");
		m_outputs[0] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[0]->setVariableName("v_Position");

        // normal
        m_comboBox->addItem("normal");
		m_outputs[1] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[1]->setVariableName("v_Normal");
        
        // color
        m_comboBox->addItem("color");
		m_outputs[2] = std::make_shared<DataVector4>(this, "vec4");
		m_outputs[2]->setVariableName("v_Color");

        // uv0
        m_comboBox->addItem("uv0");
		m_outputs[3] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[3]->setVariableName("v_UV");

        m_comboBox->addItem("uv1");
		m_outputs[4] = std::make_shared<DataVector2>(this, "vec2");
		m_outputs[4]->setVariableName("v_UV1");

        m_comboBox->addItem("tangent");
		m_outputs[5] = std::make_shared<DataVector4>(this, "vec3");
		m_outputs[5]->setVariableName("v_Tangent");

        m_comboBox->setCurrentIndex(0);

        QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onIndexChanged()));
    }

    QJsonObject InputDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        modelJson["option"] = m_comboBox->currentText().toStdString().c_str();

        return modelJson;
    }

    void InputDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["option"];
        if (!v.isUndefined())
        {
            m_comboBox->setCurrentText(v.toString());
        }
    }

    unsigned int InputDataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
        case PortType::In:      return 0;
        case PortType::Out:     return 1;
        default:                return 0;
      }
    }

    void InputDataModel::onIndexChanged()
    {
        Q_EMIT dataUpdated(0);
    }

    NodeDataType InputDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == PortType::Out ? m_outputs[index]->type() : NodeDataType{ "unknown", "Unknown" };
    }

    std::shared_ptr<NodeData> InputDataModel::outData(PortIndex portIndex)
    {
        int index = m_comboBox->currentIndex();
        return m_outputs[index];
    }

    bool InputDataModel::generateCode(ShaderCompiler& compiler)
    {
        Echo::String text = m_comboBox->currentText().toStdString().c_str();
        if (text == "position")
        {
            compiler.addMacro("ENABLE_VERTEX_POSITION");
        }
		if (text == "normal")
		{
			compiler.addMacro("ENABLE_VERTEX_NORMAL");
		}
		if (text == "color")
		{
			compiler.addMacro("ENABLE_VERTEX_COLOR");
		}
		if (text == "uv0")
		{
			compiler.addMacro("ENABLE_VERTEX_UV0");
		}
		if (text == "uv1")
		{
			compiler.addMacro("ENABLE_VERTEX_UV1");
		}
		if (text == "tangent")
		{
			compiler.addMacro("ENABLE_VERTEX_TANGENT");
		}

		return true;
    }
}
