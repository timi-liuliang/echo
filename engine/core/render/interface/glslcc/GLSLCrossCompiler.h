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
            GLES = 0,   // opengles
			GLSL,       // opengl
            MSL,        // metal
            HLSL,       // d3d
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
        // set input (glsl)
        void setInput(const char* vs, const char* fs, const char* cs);
        
        // get spirv (for vulkan)
        const vector<ui32>::type& getSPIRV(ShaderType Type);
        
        // get output shader (for opengles metal)
        std::string getOutput(ShaderLanguage language, ShaderType shaderType);
        
    private:
        // compile glsl to spirv
        void compileGlslToSpirv();

		// compile spirv to cross
        std::string compileSpirvToGles(ShaderType shaderType);
		std::string compileSpirvToMsl(ShaderType shaderType);
		std::string compileSpirvToGlsl(ShaderType shaderType);
		std::string compileSpirvToHlsl(ShaderType shaderType);

	private:
		// get preambel
		const char* getPreamble();
        
		// get processes
		const std::vector<std::string> getProcesses();

    private:
		bool				m_isNeedUpdateSpriv = true;
		bool				m_isNeedUpdateOutput= true;
        String              m_inputGlsl[ShaderType::Total];	// input shaders (glsl vulkan)
        vector<ui32>::type  m_spirv[ShaderType::Total];		// standard portabble intermediate representation
    };
}
