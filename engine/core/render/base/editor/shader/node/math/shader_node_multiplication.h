#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeMultiplication : public ShaderNode
    {
        Q_OBJECT

    public:
        ShaderNodeMultiplication();
        virtual ~ShaderNodeMultiplication() {}

		// name
		QString name() const override { return QStringLiteral("Multiplication"); }

        // caption
        QString caption() const override { return QStringLiteral("Multiplication"); }

        // is caption visible
        bool captionVisible() const override { return true; }

		// when input changed
		void setInData(std::shared_ptr<NodeData> nodeData, PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
    };
}

#endif