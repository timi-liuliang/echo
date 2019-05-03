#include "GLSLCrossCompiler.h"
#include <thirdparty/glslang/glslang/Public/ShaderLang.h>

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
        // create shader program
        glslang::TProgram* prog = EchoNew(glslang::TProgram);
        
        // deallocate program
        prog->~TProgram();
    }
    
    void GLSLCrossCompiler::compileSpirvToCross(ShaderLanguage language)
    {
        
    }
}
