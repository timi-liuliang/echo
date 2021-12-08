#include "shader_node_shared.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeShared::ShaderNodeShared()
    {
		setupWidgets();

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
    }

    void ShaderNodeShared::bindMethods()
    {
		CLASS_BIND_METHOD(ShaderNodeShared, getOption);
		CLASS_BIND_METHOD(ShaderNodeShared, setOption);

		CLASS_REGISTER_PROPERTY(ShaderNodeShared, "Shared", Variant::Type::String, getOption, setOption);
    }

    String ShaderNodeShared::getOption() const
    {
        return m_comboBox->currentText().toStdString().c_str();
    }

    void ShaderNodeShared::setOption(const String& option)
    {
        m_comboBox->setCurrentText(option.c_str());

        Q_EMIT captionUpdated();
        Q_EMIT dataUpdated(0);
    }

    unsigned int ShaderNodeShared::nPorts(QtNodes::PortType portType) const
    {
      switch (portType)
      {
        case QtNodes::PortType::In:      return 0;
        case QtNodes::PortType::Out:     return 1;
        default:                        return 0;
      }
    }

    NodeDataType ShaderNodeShared::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == QtNodes::PortType::Out ? m_outputs[index]->type() : NodeDataType{ "invalid", "invalid" };
    }

	std::shared_ptr<NodeData> ShaderNodeShared::outData(QtNodes::PortIndex portIndex)
	{
		int index = m_comboBox->currentIndex();
		return m_outputs[index];
	}

    bool ShaderNodeShared::generateCode(Echo::ShaderCompiler& compiler)
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

	void ShaderNodeShared::setupWidgets()
	{
		m_comboBox = new QComboBox();
		m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

		EditorApi.qConnectWidget(m_comboBox, QSIGNAL(currentIndexChanged(const QString&)), this, createMethodBind(&ShaderNodeShared::onComboBoxEdited));
	}

	void ShaderNodeShared::onComboBoxEdited()
	{
		setOption(m_comboBox->currentText().toStdString().c_str());
	}
}

#endif