#pragma once

#include "engine/core/render/base/shader/editor/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeVertexAttribute : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeVertexAttribute, ShaderNode)

    public:
        ShaderNodeVertexAttribute();
        virtual ~ShaderNodeVertexAttribute() {}

        // name
		virtual QString name() const override { return QStringLiteral("VertexAttribute"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Vertex Attribute"); }

        // is caption visible
        bool captionVisible() const override { return true; }

		// category
		virtual QString category() const override { return "Inputs"; }

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;

    public:
        // get port type
        virtual unsigned int nPorts(QtNodes::PortType portType) const override;

        // get data type
        virtual NodeDataType dataType(QtNodes::PortType portType, QtNodes::PortIndex portIndex) const override;

        // out data
        virtual std::shared_ptr<NodeData> outData(QtNodes::PortIndex port) override;

	public:
		// make widgets
		void setupWidgets();

		// slots
		void onComboBoxEdited();

		// get embedded widget
		QWidget* embeddedWidget() override { return m_comboBox; }

    public:
		// attribute
		String getOption() const;
        void setOption(const String& option);

    private:
        QComboBox*      m_comboBox = nullptr;
    };
}

#endif