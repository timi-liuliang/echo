#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeAbs : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeAbs, ShaderNode)

    public:
        ShaderNodeAbs();
        virtual ~ShaderNodeAbs() {}

		// name
		virtual QString name() const override { return QStringLiteral("Abs"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Abs"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(ShaderCompiler& compiler) override;
    };
}

#endif