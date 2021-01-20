#pragma once

#include "engine/core/render/base/editor/shader/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeMix : public ShaderNode
    {
        Q_OBJECT

    public:
        ShaderNodeMix();
        virtual ~ShaderNodeMix() {}

		// name
		virtual QString name() const override { return QStringLiteral("Mix"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Mix"); }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// when input changed
		virtual void setInData(std::shared_ptr<NodeData> nodeData, QtNodes::PortIndex port) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
    };
}

#endif
