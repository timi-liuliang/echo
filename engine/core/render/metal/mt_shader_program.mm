#include "mt_shader_program.h"
#include "mt_renderer.h"

static const char* testMetalLibrary = R"(
// vertex function
vertex float4 vertexShader(device float4 *vertices [[buffer(0)]], uint vid [[vertex_id]]){
    
    return vertices[vid];
    
}

// fragment function
fragment float4 fragmentShader(float4 in [[stage_in]]){
    
    //set color fragment to red
    return float4(1.0,0.0,0.0,1.0);
    
})";

namespace Echo
{
    // create shader library
    bool MTShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            NSString* sourceCode = [NSString stringWithUTF8String:testMetalLibrary];
            m_metalLibrary = [device newLibraryWithSource:sourceCode];
            
            m_metalVertexShader   = [m_metalLibrary newFunctionWithName:@"vertexShader"];
            m_metalFragmentShader = [m_metalLibrary newFunctionWithName:@"fragmentShader"];
        }
    }
}
