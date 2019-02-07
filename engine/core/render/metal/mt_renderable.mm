#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_renderer.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    VKRenderable::VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            m_metalRenderPipelineDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
            
            // assign vertex and frament shader
            MTShaderProgram* mtShaderProgram = ECHO_DOWN_CAST<MTShaderProgram*>(shader);
            if(mtShaderProgram)
            {
                [m_metalRenderPipelineDescriptor setVertexFunction:mtShaderProgram->getMetalVertexFunction()];
                [m_metalRenderPipelineDescriptor setFragmentFunction:mtShaderProgram->getMetalFragmentFunction()];
            }
            
            // specify the target-texture pixel format
            m_metalRenderPipelineDescriptor.colorAttachments[0].pixelFormat = MTLPixelFormatBGRA8Unorm;
            
            // build the rendering pipeline object
            NSError* buildError = nullptr;
            m_metalRenderPipelineState = [device newRenderPipelineStateWithDescriptor:m_metalRenderPipelineDescriptor error:&buildError];
            if(buildError)
            {
                NSString* nsError = [NSString stringWithFormat:@"%@", buildError];
                EchoLogError("%s", [nsError UTF8String]);
            }
        }
    }
}
