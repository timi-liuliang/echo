#include "GLSLCrossCompiler.h"
#include <thirdparty/glslang/glslang/Public/ShaderLang.h>
#include <thirdparty/glslang/SPIRV/SpvTools.h>
#include <thirdparty/glslang/SPIRV/GlslangToSpv.h>
#include "engine/core/log/Log.h"

namespace Echo
{
    void GLSLCrossCompiler::setInput(const char* vs, const char* fs, const char* cs)
    {
        m_inputVS = vs ? vs : "";
        m_inputFS = fs ? fs : "";
        m_inputCS = cs ? cs : "";
        
    }
    
    const vector<ui32>::type& GLSLCrossCompiler::getSPIRV()
    {
        compileGlslToSpirv();
        
        return m_spirv;
    }
    
    void GLSLCrossCompiler::compileGlslToSpirv()
    {
        // initialize process (wrong place)
        glslang::InitializeProcess();
        
        // create shader program
        glslang::TProgram* prog = EchoNew(glslang::TProgram);
        
        // shader
        const char* shaders[3] = { m_inputVS.c_str(), m_inputFS.c_str(), m_inputCS.c_str()};
        EShLanguage types[3]   = { EShLangVertex, EShLangFragment, EShLangCompute};
        for(int i=0; i<3; i++)
        {
            if(strlen(shaders[i]))
            {
                int shaderLen = strlen(shaders[i]);
                int defaultVersion = 100; // 110 for desktop
                
                glslang::TShader* shader = new glslang::TShader(types[i]);
                shader->setStringsWithLengths(&shaders[i], &shaderLen, 1);
                shader->setInvertY(false);
                shader->setEnvInput(glslang::EShSourceGlsl, types[i], glslang::EShClientVulkan, defaultVersion);
                shader->setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_1);
                shader->setEnvTarget(glslang::EShTargetSpv, glslang::EShTargetSpv_1_0);
                prog->addShader(shader);
            }
        }
        
        // link
        EShMessages messages = EShMsgDefault;
        if (prog->link(messages))
        {
            // Output and save SPIR-V for each shader
            std::vector<uint32_t> spirv;
            for (int i = 0; i < 3; i++)
            {
                glslang::TIntermediate* intermediate = prog->getIntermediate(types[i]);
                if(intermediate)
                {
                    glslang::SpvOptions spvOptions;
                    spvOptions.validate = true;
                    spv::SpvBuildLogger spvBuildLogger;
                    glslang::GlslangToSpv(*intermediate, spirv, &spvBuildLogger, &spvOptions);
                    //spvBuildLogger.getAllMessages().empty()
                }
            }
        }
        
        // delete shaders
        
        // deallocate program
        prog->~TProgram();
        
        // finalize process (wrong place)
        glslang::FinalizeProcess();
    }
    
    void GLSLCrossCompiler::compileSpirvToCross(ShaderLanguage language)
    {
        
    }
}
