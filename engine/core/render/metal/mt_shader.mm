#include "mt_shader.h"

namespace Echo
{
    MTShader::MTShader(ShaderType type, const ShaderDesc& desc, const String& filename)
        : Shader( type, desc, filename)
    {
        
    }
    
    MTShader::MTShader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size)
        : Shader(type, desc, srcBuffer, size)
    {
        
    }
    
    MTShader::~MTShader()
    {
        
    }
}
