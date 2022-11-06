#include "shader_node_direction_light_attribute.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeDirectionLightAttribute::ShaderNodeDirectionLightAttribute()
    {
        setupWidgets();

        m_outputs.clear();

        // Color
        m_comboBox->addItem("color");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Color");

        // Direction
        m_comboBox->addItem("direction");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Normal");

        m_comboBox->setCurrentIndex(0);
    }

    void ShaderNodeDirectionLightAttribute::bindMethods()
    {
		CLASS_BIND_METHOD(ShaderNodeDirectionLightAttribute, getOption);
		CLASS_BIND_METHOD(ShaderNodeDirectionLightAttribute, setOption);

		CLASS_REGISTER_PROPERTY(ShaderNodeDirectionLightAttribute, "Attribute", Variant::Type::String, getOption, setOption);
    }

    String ShaderNodeDirectionLightAttribute::getOption() const
    {
        return m_comboBox->currentText().toStdString().c_str();
    }

    void ShaderNodeDirectionLightAttribute::setOption(const String& option)
    {
        m_comboBox->setCurrentText(option.c_str());

        Q_EMIT captionUpdated();
        Q_EMIT dataUpdated(0);
    }

    unsigned int ShaderNodeDirectionLightAttribute::nPorts(QtNodes::PortType portType) const
    {
      switch (portType)
      {
        case QtNodes::PortType::In:      return 0;
        case QtNodes::PortType::Out:     return 1;
        default:                        return 0;
      }
    }

    NodeDataType ShaderNodeDirectionLightAttribute::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == QtNodes::PortType::Out ? m_outputs[index]->type() : NodeDataType{ "invalid", "invalid" };
    }

	std::shared_ptr<NodeData> ShaderNodeDirectionLightAttribute::outData(QtNodes::PortIndex portIndex)
	{
		int index = m_comboBox->currentIndex();
		return m_outputs[index];
	}

    bool ShaderNodeDirectionLightAttribute::generateCode(Echo::ShaderCompiler& compiler)
    {
        Echo::String text = m_comboBox->currentText().toStdString().c_str();

        compiler.addMacro("ENABLE_VERTEX_POSITION");

		if (Echo::StringUtil::StartWith(text, "direction"))
		{
			compiler.addMacro("ENABLE_VERTEX_NORMAL");
		}
		if (text == "color")
		{
			compiler.addMacro("ENABLE_VERTEX_COLOR");
		}

		return true;
    }

	void ShaderNodeDirectionLightAttribute::setupWidgets()
	{
		m_comboBox = new QComboBox();
		m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

		EditorApi.qConnectWidget(m_comboBox, QSIGNAL(currentIndexChanged(const QString&)), this, createMethodBind(&ShaderNodeDirectionLightAttribute::onComboBoxEdited));
	}

	void ShaderNodeDirectionLightAttribute::onComboBoxEdited()
	{
		setOption(m_comboBox->currentText().toStdString().c_str());
	}
}

#endif