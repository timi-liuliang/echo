#include "SharedUniformDataModel.h"
#include <QtCore/QJsonValue>
#include <QtGui/QDoubleValidator>
#include "DataFloat.h"
#include "DataVector2.h"
#include "DataVector3.h"
#include "DataVector4.h"

namespace DataFlowProgramming
{
    SharedUniformDataModel::SharedUniformDataModel()
      : m_comboBox(new QComboBox())
    {
        m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

        m_outputs.resize(8);

        // position
        m_comboBox->addItem("u_Time");
		m_outputs[0] = std::make_shared<DataFloat>(this, "float");
		m_outputs[0]->setVariableName("fs_ubo.u_Time");

		// position
		m_comboBox->addItem("u_CameraPosition");
		m_outputs[1] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[1]->setVariableName("fs_ubo.u_CameraPosition");

		// position
		m_comboBox->addItem("u_CameraDirection");
		m_outputs[2] = std::make_shared<DataVector3>(this, "vec3");
		m_outputs[2]->setVariableName("fs_ubo.u_CameraDirection");

		// position
		m_comboBox->addItem("u_CameraNear");
		m_outputs[3] = std::make_shared<DataFloat>(this, "float");
		m_outputs[3]->setVariableName("fs_ubo.u_CameraNear");

		// position
		m_comboBox->addItem("u_CameraFar");
		m_outputs[4] = std::make_shared<DataFloat>(this, "float");
		m_outputs[4]->setVariableName("fs_ubo.u_CameraFar");

        m_comboBox->setCurrentIndex(0);

        QObject::connect(m_comboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(onIndexChanged()));
    }

    QJsonObject SharedUniformDataModel::save() const
    {
        QJsonObject modelJson = ShaderDataModel::save();

        modelJson["option"] = m_comboBox->currentText().toStdString().c_str();

        return modelJson;
    }

    void SharedUniformDataModel::restore(QJsonObject const &p)
    {
        QJsonValue v = p["option"];
        if (!v.isUndefined())
        {
            m_comboBox->setCurrentText(v.toString());
        }
    }

    unsigned int SharedUniformDataModel::nPorts(PortType portType) const
    {
      switch (portType)
      {
        case PortType::In:      return 0;
        case PortType::Out:     return 1;
        default:                return 0;
      }
    }

    void SharedUniformDataModel::onIndexChanged()
    {
        Q_EMIT dataUpdated(0);
    }

    NodeDataType SharedUniformDataModel::dataType(PortType portType, PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == PortType::Out ? m_outputs[index]->type() : NodeDataType{ "unknown", "Unknown" };
    }

    std::shared_ptr<NodeData> SharedUniformDataModel::outData(PortIndex portIndex)
    {
        int index = m_comboBox->currentIndex();
        return m_outputs[index];
    }

    bool SharedUniformDataModel::generateCode(Echo::ShaderCompiler& compiler)
    {
        Echo::String text = m_comboBox->currentText().toStdString().c_str();
        if (text == "u_Time")
        {
            compiler.addUniform("float", text.c_str());
        }
        else if (text == "u_CameraPosition")
        {
            compiler.addUniform("vec3", text.c_str());
        }
		else if (text == "u_CameraDirection")
		{
			compiler.addUniform("vec3", text.c_str());
		}
		else if (text == "u_CameraNear")
		{
			compiler.addUniform("float", text.c_str());
		}
		else if (text == "u_CameraFar")
		{
			compiler.addUniform("float", text.c_str());
		}

		return true;
    }
}
