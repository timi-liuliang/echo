#include "GLSLCrossCompiler.h"

namespace Echo
{
    // set vs
    void GLSLCrossCompiler::setInput(const char* vs, const char* fs, const char* cs)
    {
        m_inputVS = vs;
        m_inputFS = fs;
        m_inputCS = cs;
        
    }
    
    void GLSLCrossCompiler::compileGlslToSpirv()
    {
        
    }
    
    void GLSLCrossCompiler::compileSpirvToCross(ShaderLanguage language)
    {
        
    }
}
