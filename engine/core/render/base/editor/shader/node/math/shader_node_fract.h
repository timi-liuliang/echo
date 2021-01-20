#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeFract : public ShaderNode
    {
        Q_OBJECT

    public:
        ShaderNodeFract();
        virtual ~ShaderNodeFract() {}

		// name
		virtual QString name() const override { return QStringLiteral("Fract"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Fract"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
    };
}

#endif