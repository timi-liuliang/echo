#pragma once

#include "engine/core/render/base/shader/shader_program.h"
#include "engine/core/render/base/shader/editor/node/shader_node.h"
#include "engine/core/render/base/shader/editor/compiler/shader_compiler_lighting.h"
#include "engine/core/render/base/shader/editor/compiler/shader_compiler_opaque.h"
#include "engine/core/render/base/shader/editor/compiler/shader_compiler_transparent.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
    class ShaderNodeTemplate : public ShaderNode
    {
        ECHO_VIRTUAL_CLASS(ShaderNodeTemplate, ShaderNode)

    public:
        ShaderNodeTemplate();
        virtual ~ShaderNodeTemplate() {}

        // Get compiler
        virtual ShaderCompiler* getCompiler() { return nullptr; }

        // Generate code
        virtual bool generateCode(Echo::ShaderCompiler& compiler) override { return false; }

    private:
    };
}

#endif