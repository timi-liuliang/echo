#pragma once

#include "engine/core/render/base/shader/editor/node/shader_node.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeTemplate : public ShaderNode
    {
        ECHO_CLASS(ShaderNodeTemplate, ShaderNode)

    public:
        ShaderNodeTemplate();
        virtual ~ShaderNodeTemplate() {}

		// name
		virtual QString name() const override { return QStringLiteral("ShaderTemplate"); }

        // caption
        virtual QString caption() const override { return QStringLiteral("Shader Template"); }

		// category
		virtual QString category() const override { return "skip me"; }

        // is caption visible
        virtual bool captionVisible() const override { return true; }

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;
    };
}

#endif