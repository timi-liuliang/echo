#include "GLSLCrossCompiler.h"

namespace Echo
{
    // set vs
    void GLSLCrossCompiler::setInput(const char* vs, const char* fs, const char* cs)
    {
        m_inputVS = vs ? vs : "";
        m_inputFS = fs ? fs : "";
        m_inputCS = cs ? cs : "";
        
    }
    
    void GLSLCrossCompiler::compileGlslToSpirv()
    {
        
    }
    
    void GLSLCrossCompiler::compileSpirvToCross(ShaderLanguage language)
    {
        
    }
}
