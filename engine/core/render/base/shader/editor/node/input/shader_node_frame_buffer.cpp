#include "shader_node_frame_buffer.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    ShaderNodeFrameBuffer::ShaderNodeFrameBuffer()
    {
        setupWidgets();

        m_outputs.clear();

        // position
        m_comboBox->addItem("position(local)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Position.local");

        m_comboBox->addItem("position(world)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Position.world");

        m_comboBox->addItem("position(view)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Position.view");

        // normal
        m_comboBox->addItem("normal(local)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_NormalLocal");

        m_comboBox->addItem("normal(world)");
        m_outputs.emplace_back(std::make_shared<DataVector3>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Normal");

        // color
        m_comboBox->addItem("color");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Color");

        // uv0
        m_comboBox->addItem("uv0");
        m_outputs.emplace_back(std::make_shared<DataVector2>(this, "vec2"));
        m_outputs.back()->setVariableName("v_UV");

        m_comboBox->addItem("uv1");
        m_outputs.emplace_back(std::make_shared<DataVector2>(this, "vec2"));
        m_outputs.back()->setVariableName("v_UV1");

        m_comboBox->addItem("tangent");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec3"));
        m_outputs.back()->setVariableName("v_Tangent");

        m_comboBox->addItem("weights");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Weight");

        m_comboBox->addItem("joints");
        m_outputs.emplace_back(std::make_shared<DataVector4>(this, "vec4"));
        m_outputs.back()->setVariableName("v_Joint");

        m_comboBox->setCurrentIndex(0);
    }

    void ShaderNodeFrameBuffer::bindMethods()
    {
        CLASS_BIND_METHOD(ShaderNodeFrameBuffer, getOption, DEF_METHOD("getOption"));
        CLASS_BIND_METHOD(ShaderNodeFrameBuffer, setOption, DEF_METHOD("setOption"));

        CLASS_REGISTER_PROPERTY(ShaderNodeFrameBuffer, "Attribute", Variant::Type::String, "getOption", "setOption");
    }

    String ShaderNodeFrameBuffer::getOption() const
    {
        return m_comboBox->currentText().toStdString().c_str();
    }

    void ShaderNodeFrameBuffer::setOption(const String& option)
    {
        m_comboBox->setCurrentText(option.c_str());

        Q_EMIT captionUpdated();
        Q_EMIT dataUpdated(0);
    }

    unsigned int ShaderNodeFrameBuffer::nPorts(QtNodes::PortType portType) const
    {
        switch (portType)
        {
        case QtNodes::PortType::In:      return 0;
        case QtNodes::PortType::Out:     return 1;
        default:                        return 0;
        }
    }

    NodeDataType ShaderNodeFrameBuffer::dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const
    {
        int index = m_comboBox->currentIndex();
        return portType == QtNodes::PortType::Out ? m_outputs[index]->type() : NodeDataType{ "invalid", "invalid" };
    }

    std::shared_ptr<NodeData> ShaderNodeFrameBuffer::outData(QtNodes::PortIndex portIndex)
    {
        int index = m_comboBox->currentIndex();
        return m_outputs[index];
    }

    bool ShaderNodeFrameBuffer::generateCode(Echo::ShaderCompiler& compiler)
    {
        Echo::String text = m_comboBox->currentText().toStdString().c_str();
        if (Echo::StringUtil::StartWith(text, "position("))
        {
            compiler.addMacro("ENABLE_VERTEX_POSITION");
        }
        if (Echo::StringUtil::StartWith(text, "normal("))
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
        if (text == "weights" || text == "joints")
        {
            compiler.addMacro("ENABLE_VERTEX_BLENDING");
        }

        return true;
    }

    void ShaderNodeFrameBuffer::setupWidgets()
    {
        m_comboBox = new QComboBox();
        m_comboBox->setMinimumWidth(m_comboBox->sizeHint().width() * 1.7);

        EditorApi.qConnectWidget(m_comboBox, QSIGNAL(currentIndexChanged(const QString&)), this, createMethodBind(&ShaderNodeFrameBuffer::onComboBoxEdited));
    }

    void ShaderNodeFrameBuffer::onComboBoxEdited()
    {
        setOption(m_comboBox->currentText().toStdString().c_str());
    }
}

#endif