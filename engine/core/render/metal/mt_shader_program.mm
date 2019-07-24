#include "mt_shader_program.h"
#include "mt_renderer.h"
#include "mt_mapping.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    static boolean_t createShader(const String& content, id<MTLLibrary>& oLibrary, id<MTLFunction>& oFunction)
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            NSString* sourceCode = [NSString stringWithUTF8String:content.c_str()];
            MTLCompileOptions* compileOptions = [MTLCompileOptions new];
            compileOptions.languageVersion = MTLLanguageVersion1_1;
            NSError* compileError = nullptr;
            oLibrary = [device newLibraryWithSource:sourceCode options:compileOptions error:&compileError];
            if(!compileError)
            {
                oFunction   = [oLibrary newFunctionWithName:@"main0"];

                return oFunction ? true : false;
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
    
    // create shader library
    // https://fuchsia.googlesource.com/third_party/glfw/+/70297aeb493541072545760c379dd170ba54acbb/examples/metal.m
    bool MTShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
    {
        bool isCreateVSSucceed = createShader(vsContent, m_metalVertexLibrary, m_metalVertexShader);
        bool isCreatePSSucceed = createShader(psContent, m_metalFragmentLibrary, m_metalFragmentShader);
        m_isValid = isCreateVSSucceed && isCreatePSSucceed;
        
        return m_isValid;
    }
    
    // reference https://github.com/bkaradzic/bgfx/issues/960
    void MTShaderProgram::parseUniforms(MTLRenderPipelineReflection* reflection)
    {
        if(reflection)
        {
            m_uniforms.clear();
            
            // vertex arguments
            for(i32 i=0; i<reflection.vertexArguments.count; i++)
                addUniform(reflection.vertexArguments[i]);
            
            // fragment arguments
            for(i32 i=0; i<reflection.fragmentArguments.count; i++)
                addUniform( reflection.fragmentArguments[i]);
        }
    }
    
    void MTShaderProgram::addUniform(MTLArgument* arg)
    {
        MTLStructType* structType = arg.bufferStructType;
        for(i32 i=0; i<structType.members.count; i++)
        {
            MTLStructMember* member = structType.members[i];
            MTLArrayType*    arrayInfo = member.arrayType;

            Uniform desc;
            desc.m_name = [member.name UTF8String];
            desc.m_type = MTMapping::MapUniformType( arrayInfo ? arrayInfo.dataType : member.dataType);
            desc.m_count = arrayInfo ? arrayInfo.arrayLength : 1;
            desc.m_sizeInBytes = desc.m_count * getUniformByteSizeByUniformType(desc.m_type);
            desc.m_location = member.offset;
            m_uniforms[desc.m_location] = desc;
        }
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
