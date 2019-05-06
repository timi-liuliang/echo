#pragma once

#include <string>
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

		// ShaderType
		enum ShaderType
		{
			VS = 0,
			FS,
			CS,
			Total,
		};
        
    public:
        // set vs
        void setInput(const char* vs, const char* fs, const char* cs);
        
        // get spirv
        const vector<ui32>::type& getSPIRV(ShaderType Type);
        
    private:
        // compile
        void compileGlslToSpirv();
        void compileSpirvToCross(ShaderLanguage language);

	private:
		// get preambel
		const char* getPreamble();
        
		// get processes
		const std::vector<std::string> getProcesses();

    private:
        String              m_inputGlsl[ShaderType::Total];	// vertex shader
        vector<ui32>::type  m_spirv[ShaderType::Total];		// standard portabble intermediate representation
    };
}
