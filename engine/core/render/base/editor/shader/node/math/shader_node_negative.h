#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeNegative : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeNegative, ShaderNode)

    public:
        ShaderNodeNegative();
        virtual ~ShaderNodeNegative() {}

		// name
		virtual QString name() const override { return QStringLiteral("Negative"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Negative"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// category
		virtual QString category() const override { return "Math"; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;
    };
}

#endif