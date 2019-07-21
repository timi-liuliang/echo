#include "mt_shader_program.h"
#include "mt_renderer.h"
#include "engine/core/log/Log.h"

static const char* testMetalLibrary = R"(
#include <metal_stdlib>

using namespace metal;

struct VertexIn
{
    float3 a_Position [[ attribute(0) ]];
    float4 a_Color [[ attribute(1) ]];
};

struct RasterizerData
{
    float4 v_Position [[ position ]];
    float4 v_Color;
};

struct UBO
{
    float alphaScale;
};

// vertex function
vertex RasterizerData vertexShader(const VertexIn vIn [[ stage_in ]], constant UBO& ubo [[ buffer(0) ]])
{
    RasterizerData rd;
    rd.v_Position = float4(vIn.a_Position, 1.0);
    rd.v_Color = vIn.a_Color * ubo.alphaScale;
    
    return rd;
}

// fragment function
fragment float4 fragmentShader(RasterizerData rd [[stage_in]])
{
    return rd.v_Color;
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
                
                parseUniforms();
                
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
    
    void MTShaderProgram::parseUniforms()
    {
        
    }
    
    void MTShaderProgram::bindUniforms()
    {
        for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            Uniform& uniform = it->second;
            if (uniform.m_value)
            {
                if (uniform.m_isDirty && uniform.m_type != SPT_UNKNOWN)
                {
                    switch (uniform.m_type)
                    {
                        case SPT_VEC4:      break;
                        case SPT_MAT4:      break;
                        case SPT_INT:       break;
                        case SPT_FLOAT:     break;
                        case SPT_VEC2:      break;
                        case SPT_VEC3:      break;
                        case SPT_TEXTURE:   break;
                        default:            EchoAssertX(0, "unknow shader param format!");                                                    break;
                    }
                    
                    uniform.m_isDirty = false;
                }
            }
            else
            {
                EchoLogError("Shader param is NULL, Material");
            }
        }
    }
    
    void MTShaderProgram::bindRenderable(Renderable* renderable)
    {
        
    }
}
