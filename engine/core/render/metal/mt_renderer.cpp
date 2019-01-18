#include "mt_renderer.h"
#include "mt_frame_buffer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"
#include "mt_render_target.h"

namespace Echo
{
    MTRenderer::MTRenderer()
    {
        
    }
    
    MTRenderer::~MTRenderer()
    {
        
    }

    bool MTRenderer::initialize(const Config& config)
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
    
	void MTRenderer::setViewport(Viewport* pViewport)
	{

	}

	void MTRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}
    
    Renderable* MTRenderer::createRenderable(const String& renderStage, ShaderProgram* material)
    {
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;
        
        return renderable;
    }
    
    ShaderProgram* MTRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }
    
    // create states
    RasterizerState* MTRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(VKRasterizerState);
    }
    
    DepthStencilState* MTRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
    {
        return nullptr;
        //return EchoNew(VKDepthStencilState);
    }
    
    BlendState* MTRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(VKBlendState);
    }
    
    const SamplerState* MTRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(VKSamplerState);
    }
    
    RenderTarget* MTRenderer::createRenderTarget(ui32 id, ui32 width, ui32 height, PixelFormat pixelFormat, const RenderTarget::Options& option)
    {
        return EchoNew(MTRenderTarget);
    }
}
