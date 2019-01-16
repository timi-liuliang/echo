#include "mt_renderer.h"
#include "mt_frame_buffer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"

namespace Echo
{
    VKRenderer::VKRenderer()
    {
        
    }
    
    VKRenderer::~VKRenderer()
    {
        
    }

    bool VKRenderer::initialize(const Config& config)
    {
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;
        
        m_frameBuffer = EchoNew(VKFrameBuffer);
        
        // set view port
        Viewport* pViewport = EchoNew(Viewport(0, 0, m_screenWidth, m_screenHeight));
        m_frameBuffer->setViewport(pViewport);
        setViewport(pViewport);
        
        return true;
    }
    
	void VKRenderer::setViewport(Viewport* pViewport)
	{

	}

	void VKRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}
    
    Renderable* VKRenderer::createRenderable(const String& renderStage, ShaderProgram* material)
    {
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;
        
        return renderable;
    }
    
    ShaderProgram* VKRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }
    
    // create states
    RasterizerState* VKRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(VKRasterizerState);
    }
    
    DepthStencilState* VKRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
    {
        return nullptr;
        //return EchoNew(VKDepthStencilState);
    }
    
    BlendState* VKRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(VKBlendState);
    }
    
    const SamplerState* VKRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(VKSamplerState);
    }
}
