#include "mt_shader_program.h"
#include "mt_renderer.h"
#include "engine/core/log/Log.h"

static const char* testMetalLibrary = R"(
#include <metal_stdlib>

using namespace metal;

struct VertexIn
{
    float3 position;
    float4 color;
};

// vertex function
vertex float4 vertexShader(device VertexIn* vertices [[buffer(0)]], uint vid [[vertex_id]])
{
    return float4(vertices[vid].position, 1.0);
}

// fragment function
fragment float4 fragmentShader(float4 in [[stage_in]])
{
    //set color fragment to red
    return float4(1.0,0.0,0.0,1.0);
}
)";

namespace Echo
{
    // create shader library
    // https://fuchsia.googlesource.com/third_party/glfw/+/70297aeb493541072545760c379dd170ba54acbb/examples/metal.m
    bool MTShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            NSString* sourceCode = [NSString stringWithUTF8String:testMetalLibrary];
            MTLCompileOptions* compileOptions = [MTLCompileOptions new];
            compileOptions.languageVersion = MTLLanguageVersion1_1;
            NSError* compileError = nullptr;
            m_metalLibrary = [device newLibraryWithSource:sourceCode options:compileOptions error:&compileError];
            if(!compileError)
            {
                m_metalVertexShader   = [m_metalLibrary newFunctionWithName:@"vertexShader"];
                m_metalFragmentShader = [m_metalLibrary newFunctionWithName:@"fragmentShader"];
                
                return m_metalVertexShader && m_metalFragmentShader ? true : false;
            }
            else
            {
                NSString* nsError = [NSString stringWithFormat:@"%@", compileError];
                EchoLogError("%s", [nsError UTF8String]);
                
                return false;
            }
        }
        
        return false;
    }
}
