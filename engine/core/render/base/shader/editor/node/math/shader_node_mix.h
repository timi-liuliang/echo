#pragma once

#include "engine/core/render/base/shader/editor/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeMix : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeMix, ShaderNode)

    public:
        ShaderNodeMix();
        virtual ~ShaderNodeMix() {}

		// name
		virtual QString name() const override { return QStringLiteral("Mix"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Mix"); }

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
