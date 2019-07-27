#include "engine/core/render/interface/mesh/Mesh.h"
#include "mt_mapping.h"
#include "mt_gpu_buffer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_gpu_buffer.h"
#include "mt_renderer.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    MTRenderable::MTRenderable(const String& renderStage, ShaderProgram* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {
        // assign vertex and frament shader
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(shader);
        if(mtShaderProgram && mtShaderProgram->isValid())
        {
            m_metalRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
            [m_metalRenderPipelineDescriptor setVertexFunction:mtShaderProgram->getMetalVertexFunction()];
            [m_metalRenderPipelineDescriptor setFragmentFunction:mtShaderProgram->getMetalFragmentFunction()];
            
            // specify the target-texture pixel format
            m_metalRenderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
        }
    }
    
    void MTRenderable::buildRenderPipelineState()
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device && m_metalRenderPipelineDescriptor)
        {
            // clear
            if(m_metalRenderPipelineState)
                [m_metalRenderPipelineState release];
            
            // create new one
            NSError* buildError = nullptr;
            m_metalRenderPipelineState = [device newRenderPipelineStateWithDescriptor:m_metalRenderPipelineDescriptor options:MTLPipelineOptionArgumentInfo reflection:&m_metalRenderPipelineReflection error:&buildError];
            if(!buildError)
            {
                MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_shaderProgram.ptr());
                mtShaderProgram->parseUniforms(m_metalRenderPipelineReflection);
            }
            else
            {
                NSString* nsError = [NSString stringWithFormat:@"%@", buildError];
                EchoLogError("%s", [nsError UTF8String]);
            }
        }
    }
    
    id<MTLBuffer> MTRenderable::getMetalIndexBuffer()
    {
        MTBuffer* mtBuffer = ECHO_DOWN_CAST<MTBuffer*>(m_mesh->getIndexBuffer());
        return mtBuffer->getMetalBuffer();
    }
    
    id<MTLBuffer> MTRenderable::getMetalVertexBuffer()
    {
        MTBuffer* mtBuffer = ECHO_DOWN_CAST<MTBuffer*>(m_mesh->getVertexBuffer());
        return mtBuffer->getMetalBuffer();
    }
    
    void MTRenderable::link()
    {
        bindVertexStream(m_mesh->getVertexElements(), m_mesh->getVertexBuffer());
    }
    
    bool MTRenderable::bindVertexStream(const VertexElementList& vertElements, GPUBuffer* vertexBuffer, int flag)
    {
        if (flag & BS_BEGIN)
            m_vertexStreams.clear();
        
        StreamUnit unit;
        unit.m_vertElements = vertElements;
        unit.m_buffer = vertexBuffer;
        buildVertexDescriptor(&unit);
        
        m_vertexStreams.push_back(unit);
        
        return true;
    }
    
    MTLVertexAttribute* MTRenderable::getMTLVertexAttributeBySemantic(VertexSemantic semantic)
    {
        String attributeName = MTMapping::MapVertexSemanticString(semantic);
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_shaderProgram.ptr());
        if(mtShaderProgram && mtShaderProgram->isValid())
        {
            id<MTLFunction> vertexShader = mtShaderProgram->getMetalVertexFunction();
            for(i32 idx=0; idx<vertexShader.vertexAttributes.count; idx++)
            {
                MTLVertexAttribute* attribute = vertexShader.vertexAttributes[idx];
                if([attribute.name UTF8String] == attributeName)
                    return attribute;
            }
        }
        
        return nullptr;
    }
    
    void MTRenderable::buildVertexDescriptor(StreamUnit* stream)
    {
        if( m_metalVertexDescriptor)
            [m_metalVertexDescriptor release];
        
        m_metalVertexDescriptor = [[MTLVertexDescriptor alloc] init];
        
        // iterate all elements
        ui32 numVertElms = static_cast<ui32>(stream->m_vertElements.size());
        if (numVertElms > 0)
        {
            // buffer 0 was used by uniform buffer
            const i32 bufferIdx = 1;
            ui32 elementOffset = 0;
            for (size_t i = 0; i < numVertElms; ++i)
            {
                MTLVertexAttribute* mtlAttribute = getMTLVertexAttributeBySemantic(stream->m_vertElements[i].m_semantic);
                if(mtlAttribute)
                {
                    ui32 attributeIdx = mtlAttribute.attributeIndex;
                    m_metalVertexDescriptor.attributes[attributeIdx].format = MTMapping::MapVertexFormat(stream->m_vertElements[i].m_pixFmt);
                    m_metalVertexDescriptor.attributes[attributeIdx].bufferIndex = bufferIdx;
                    m_metalVertexDescriptor.attributes[attributeIdx].offset = elementOffset;
                }
                
                elementOffset += PixelUtil::GetPixelSize(stream->m_vertElements[i].m_pixFmt);
            }
            
            m_metalVertexDescriptor.layouts[bufferIdx].stride = elementOffset;
            [m_metalRenderPipelineDescriptor setVertexDescriptor:m_metalVertexDescriptor];
            
            // update render pipeline state
            buildRenderPipelineState();
        }
    }
    
    void MTRenderable::bindShaderParams()
    {
        bindTextures();
        
        if(m_shaderProgram)
        {
            for(auto& it : m_shaderParams)
            {
                ShaderParam& uniform = it.second;
                m_shaderProgram->setUniform( uniform.name.c_str(), uniform.data, uniform.type, uniform.length);
            }
        }
    }
}
