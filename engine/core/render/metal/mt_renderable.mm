#include "engine/core/render/base/mesh/Mesh.h"
#include "mt_mapping.h"
#include "mt_gpu_buffer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_gpu_buffer.h"
#include "mt_renderer.h"
#include "mt_shader_program.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    MTRenderable::MTRenderable(const String& renderStage, ShaderProgram* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {
    }

    void MTRenderable::buildRenderPipelineState()
    {
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_shaderProgram.ptr());
        if(mtShaderProgram && mtShaderProgram->isValid())
        {
            MTLRenderPipelineDescriptor* mtRenderPipelineDescriptor = mtShaderProgram->getMTRenderPipelineDescriptor();
            id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
            if(device && mtRenderPipelineDescriptor)
            {
                // clear
                if(m_metalRenderPipelineState)
                    [m_metalRenderPipelineState release];
                
                [mtRenderPipelineDescriptor setVertexDescriptor:m_metalVertexDescriptor];

                // create new one
                NSError* buildError = nullptr;
                m_metalRenderPipelineState = [device newRenderPipelineStateWithDescriptor:mtRenderPipelineDescriptor options:MTLPipelineOptionArgumentInfo reflection:nil error:&buildError];
                if(buildError)
                {
                    NSString* nsError = [NSString stringWithFormat:@"%@", buildError];
                    EchoLogError("%s", [nsError UTF8String]);
                }
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

    void MTRenderable::setMesh(Mesh* mesh)
    {
        m_mesh = mesh;

        buildVertexDescriptor();
    }

    void MTRenderable::buildVertexDescriptor()
    {
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_shaderProgram.ptr());
        if(mtShaderProgram && mtShaderProgram->isValid())
        {
            if( m_metalVertexDescriptor)
                [m_metalVertexDescriptor release];
            
            m_metalVertexDescriptor = mtShaderProgram->buildVertexDescriptor( m_mesh->getVertexElements());
            
            // update render pipeline state
            buildRenderPipelineState();
        }
    }

    void MTRenderable::bindShaderParams()
    {
        MTShaderProgram* mtShaderProgram = dynamic_cast<MTShaderProgram*>(m_shaderProgram.ptr());
        if(mtShaderProgram)
        {
            for(auto& it : m_shaderParams)
            {
                ShaderParam& uniform = it.second;
                mtShaderProgram->setUniform( uniform.name.c_str(), uniform.data, uniform.type, uniform.length);
            }

            mtShaderProgram->bindUniforms(this);
        }
    }
}
