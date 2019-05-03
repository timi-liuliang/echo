#pragma once

#include "engine/core/util/StringUtil.h"

namespace Echo
{
    /**
     * GLSL cross-compiler tool (GLSL->HLSL, MSL, GLES2, GLES3, GLSLv3), using SPIRV-cross and glslang
     * [1]. septag(2019)-glslcc : https://github.com/septag/glslcc
     */
    class GLSLCrossCompiler
    {
    public:
        enum ShaderLanguage
        {
            GLES = 0,
            MSL,
            HLSL,
            GLSL,
        };
        
    public:
        // set vs
        void setInput(const char* vs, const char* fs, const char* cs);
        
        // get spirv
        const vector<ui32>::type& getSPIRV();
        
    private:
        // compile
        void compileGlslToSpirv();
        void compileSpirvToCross(ShaderLanguage language);
        
    private:
        String              m_inputVS;      // vertex shader
        String              m_inputFS;      // fragment shader
        String              m_inputCS;      // compute shader
        vector<ui32>::type  m_spirv;        // standard portabble intermediate representation
    };
}
