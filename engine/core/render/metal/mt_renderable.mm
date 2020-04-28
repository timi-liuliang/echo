#include "engine/core/render/base/mesh/Mesh.h"
#include "engine/core/scene/render_node.h"
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
    MTRenderable::MTRenderable(int identifier)
        : Renderable(identifier)
    {
    }

    void MTRenderable::buildRenderPipelineState()
    {
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_material->getShader());
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
        MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_material->getShader());
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
        MTShaderProgram* shaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(m_material->getShader());
        if(shaderProgram)
        {
            i32 textureCount = 0;
            for(auto& it : shaderProgram->getUniforms())
            {
                ShaderProgram::UniformPtr uniform = it.second;
                Material::UniformValue* uniformValue = m_material->getUniform(uniform->m_name);
                if (uniform->m_type != SPT_TEXTURE)
                {
                    const void* value = m_node ? m_node->getGlobalUniformValue(uniform->m_name) : nullptr;
                    if (!value) value = uniformValue->getValue();

                    shaderProgram->setUniform(uniform->m_name.c_str(), value, uniform->m_type, uniform->m_count);
                }
                else
                {
                    if(uniformValue)
                    {
                        Texture* texture = uniformValue->getTexture();
                        Renderer::instance()->setTexture(textureCount, texture);
                    }

                    shaderProgram->setUniform(uniform->m_name.c_str(), &textureCount, uniform->m_type, uniform->m_count);
                    textureCount++;
                }
            }

            shaderProgram->bindUniforms(this);
        }
    }
}
