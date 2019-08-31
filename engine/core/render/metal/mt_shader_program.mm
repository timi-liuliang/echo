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
                addUniform(reflection.vertexArguments[i], ShaderType::VS);

            // fragment arguments
            for(i32 i=0; i<reflection.fragmentArguments.count; i++)
                addUniform( reflection.fragmentArguments[i], ShaderType::FS);

            allocUniformBytes();
        }
    }

    void MTShaderProgram::addUniform(MTLArgument* arg, ShaderType shaderType)
    {
        MTLStructType* structType = arg.bufferStructType;
        for(i32 i=0; i<structType.members.count; i++)
        {
            MTLStructMember* member = structType.members[i];
            MTLArrayType*    arrayInfo = member.arrayType;

            Uniform desc;
            desc.m_name = [member.name UTF8String];
            desc.m_shader = shaderType;
            desc.m_type = MTMapping::MapUniformType( arrayInfo ? arrayInfo.dataType : member.dataType);
            desc.m_count = arrayInfo ? arrayInfo.arrayLength : 1;
            desc.m_sizeInBytes = desc.m_count * MapUniformTypeSize(desc.m_type);
            desc.m_location = member.offset;
            m_uniforms[desc.m_name] = desc;
        }
    }

    void MTShaderProgram::allocUniformBytes()
    {
        m_vertexShaderUniformBytes.clear();
        m_fragmentShaderUniformBytes.clear();

        for(auto& it : m_uniforms)
        {
            const Uniform& uniform = it.second;
            vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
            i32 bytes = uniform.m_location + uniform.m_sizeInBytes;
            while(uniformBytes.size()<bytes)
            {
                uniformBytes.push_back(0);
            }
        }
    }

    void MTShaderProgram::bindUniforms()
    {
        // organize uniform bytes
        for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            Uniform& uniform = it->second;
            if (uniform.m_value && uniform.m_type != SPT_UNKNOWN)
            {
                vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
                if(uniform.m_type != SPT_TEXTURE)
                {
                    std::memcpy(uniformBytes.data()+uniform.m_location, uniform.m_value, uniform.m_sizeInBytes*sizeof(Byte));
                }
                else
                {

                }
            }
        }

        // set uniforms
        MTRenderer* render = ECHO_DOWN_CAST<MTRenderer*>(Renderer::instance());
        if(render)
        {
            id<MTLRenderCommandEncoder> commandEncoder = render->getMetalRenderCommandEncoder();
            if(m_vertexShaderUniformBytes.size())
                [commandEncoder setVertexBytes:m_vertexShaderUniformBytes.data() length:m_vertexShaderUniformBytes.size() atIndex:0];

            if(m_fragmentShaderUniformBytes.size())
                [commandEncoder setFragmentBytes:m_fragmentShaderUniformBytes.data() length:m_fragmentShaderUniformBytes.size() atIndex:0];
        }
    }
}
