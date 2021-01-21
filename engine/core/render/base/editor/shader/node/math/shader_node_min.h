#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeMin : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeMin, ShaderNode)

    public:
        ShaderNodeMin();
        virtual ~ShaderNodeMin() {}

		// name
		virtual QString name() const override { return QStringLiteral("Min"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Min"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// category
		virtual QString category() const override { return "Math"; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
    };
}

#endif
