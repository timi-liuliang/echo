#pragma once

#include "engine/core/render/base/shader/shader_program.h"
#include "engine/core/render/base/shader/editor/node/shader_node.h"
#include "engine/core/render/base/shader/editor/compiler/shader_compiler_lighting.h"
#include "engine/core/render/base/shader/editor/compiler/shader_compiler_surface.h"

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

    public:
        // Get compiler
        ShaderCompiler* getCompiler();

    public:
        // Domain
        void setDomain(const ShaderProgram::Domain domain);

    private:
        ShaderProgram::Domain   m_domain = ShaderProgram::Domain::Surface;
        ShaderCompilerSurface   m_compilerSurface;
        ShaderCompilerLighting  m_compilerLighting;
    };
}

#endif