#include "mt_shader_program.h"
#include "mt_renderable.h"
#include "mt_renderer.h"
#include "mt_mapping.h"
#include "mt_texture.h"
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
        
        if(m_isValid)
            parseUniforms();

        return m_isValid;
    }

    MTLVertexAttribute* MTShaderProgram::getMTLVertexAttributeBySemantic(VertexSemantic semantic)
    {
        String attributeName = MTMapping::MapVertexSemanticString(semantic);
        if(isValid())
        {
            id<MTLFunction> vertexShader = getMetalVertexFunction();
            for(i32 idx=0; idx<vertexShader.vertexAttributes.count; idx++)
            {
                MTLVertexAttribute* attribute = vertexShader.vertexAttributes[idx];
                if([attribute.name UTF8String] == attributeName)
                    return attribute;
            }
        }

        return nullptr;
    }

    MTLVertexDescriptor* MTShaderProgram::buildVertexDescriptor(const VertexElementList& vertElments)
    {
        MTLVertexDescriptor* metalVertexDescriptor = [[MTLVertexDescriptor alloc] init];

        // iterate all elements
        ui32 numVertElms = static_cast<ui32>(vertElments.size());
        if (numVertElms > 0)
        {
            // buffer 0 was used by uniform buffer
            const i32 bufferIdx = 1;
            ui32 elementOffset = 0;
            for (size_t i = 0; i < numVertElms; ++i)
            {
                MTLVertexAttribute* mtlAttribute = getMTLVertexAttributeBySemantic(vertElments[i].m_semantic);
                if(mtlAttribute)
                {
                    ui32 attributeIdx = mtlAttribute.attributeIndex;
                    metalVertexDescriptor.attributes[attributeIdx].format = MTMapping::MapVertexFormat(vertElments[i].m_pixFmt);
                    metalVertexDescriptor.attributes[attributeIdx].bufferIndex = bufferIdx;
                    metalVertexDescriptor.attributes[attributeIdx].offset = elementOffset;
                }

                elementOffset += PixelUtil::GetPixelSize(vertElments[i].m_pixFmt);
            }

            metalVertexDescriptor.layouts[bufferIdx].stride = elementOffset;
        }
        
        return metalVertexDescriptor;
    }

    void MTShaderProgram::parseUniforms()
    {
        // Init Descriptor
        m_metalRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
        [m_metalRenderPipelineDescriptor setVertexFunction:getMetalVertexFunction()];
        [m_metalRenderPipelineDescriptor setFragmentFunction:getMetalFragmentFunction()];

        // specify the target-texture pixel format
        m_metalRenderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        
        // Specify vertex descriptor
        MeshVertexFormat define;
        define.m_isUseNormal = true;
        define.m_isUseVertexColor = true;
        define.m_isUseUV = true;
        define.m_isUseLightmapUV = true;
        define.m_isUseBoneData = true;
        define.m_isUseTangentBinormal = true;
        define.build();
        
        MTLVertexDescriptor* mtVertexDescriptor = buildVertexDescriptor( define.m_vertexElements);
        [m_metalRenderPipelineDescriptor setVertexDescriptor:mtVertexDescriptor];
        
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device && m_metalRenderPipelineDescriptor)
        {
            NSError* buildError = nullptr;
            id<MTLRenderPipelineState> mtRenderPipelineState = [device newRenderPipelineStateWithDescriptor:m_metalRenderPipelineDescriptor options:MTLPipelineOptionArgumentInfo reflection:&m_metalRenderPipelineReflection error:&buildError];
            if(!buildError)
            {
                parseUniforms(m_metalRenderPipelineReflection);
                
                [mtRenderPipelineState release];
                [mtVertexDescriptor release];
            }
        }
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
        if(arg.type == MTLArgumentTypeBuffer)
        {
            MTLStructType* structType = arg.bufferStructType;
            if(structType)
            {
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
        }
        else if(arg.type == MTLArgumentTypeTexture)
        {
            Uniform desc;
            desc.m_name = [arg.name UTF8String];
            desc.m_shader = shaderType;
            desc.m_type = MTMapping::MapUniformType(MTLDataTypeTexture);
            desc.m_count = 1;
            desc.m_sizeInBytes = 4;
            desc.m_location = arg.index;
            
            m_uniforms[desc.m_name] = desc;
        }
        else if(arg.type == MTLArgumentTypeSampler)
        {
//            Uniform desc;
//            desc.m_name = [arg.name UTF8String];
//            desc.m_shader = shaderType;
//            desc.m_type = MTMapping::MapUniformType(MTLDataTypeSampler);
//            desc.m_count = 1;
//            desc.m_sizeInBytes = 4;
//            desc.m_location = arg.index;
//
//            m_uniforms[desc.m_name] = desc;
        }
        else
        {
            EchoLogError("UnProcessored MTLArgumentType %d", arg.type);
        }
    }

    void MTShaderProgram::allocUniformBytes()
    {
        m_vertexShaderUniformBytes.clear();
        m_fragmentShaderUniformBytes.clear();

        for(auto& it : m_uniforms)
        {
            const Uniform& uniform = it.second;
            if(uniform.m_type!=SPT_TEXTURE)
            {
                vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
                i32 bytes = uniform.m_location + uniform.m_sizeInBytes;
                while(uniformBytes.size()<bytes)
                {
                    uniformBytes.push_back(0);
                }
            }
        }
    }

    void MTShaderProgram::bindUniforms(MTRenderable* renderable)
    {
        // organize uniform bytes
        for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
        {
            Uniform& uniform = it->second;
            if (uniform.m_value && uniform.m_type != SPT_UNKNOWN && uniform.m_type!=SPT_TEXTURE)
            {
                vector<Byte>::type& uniformBytes = uniform.m_shader == ShaderType::VS ? m_vertexShaderUniformBytes : m_fragmentShaderUniformBytes;
                if(uniform.m_type != SPT_TEXTURE)
                {
                    std::memcpy(uniformBytes.data()+uniform.m_location, uniform.m_value, uniform.m_sizeInBytes*sizeof(Byte));
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
            
            // textues
            for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
            {
                Uniform& uniform = it->second;
                if (uniform.m_shader == ShaderType::FS && uniform.m_type == SPT_TEXTURE)
                {
                    i32 texSlot = *(i32*)(uniform.m_value);
                    MTTexture2D* texture = ECHO_DOWN_CAST<MTTexture2D*>(renderable->getTexture(texSlot));
                    if(texture)
                    {
                        [commandEncoder setFragmentSamplerState:texture->getMTSamplerState() atIndex:uniform.m_location];
                        [commandEncoder setFragmentTexture:texture->getMTTexture() atIndex:uniform.m_location];
                    }
                }
            }
        }
    }
}
