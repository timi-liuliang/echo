#include "mt_renderer.h"
#include "mt_renderable.h"
#include "mt_shader_program.h"
#include "mt_render_state.h"
#include "mt_render_target.h"
#include "mt_texture.h"
#include <AppKit/AppKit.h>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

namespace Echo
{
    struct MTStruct
    {
        // The device (aka GPU) we're using to render
        id<MTLDevice> device;
    };
    
    MTRenderer::MTRenderer()
    {
        m_struct = EchoNew(MTStruct);
        m_struct->device = MTLCreateSystemDefaultDevice();
    }
    
    MTRenderer::~MTRenderer()
    {
        
    }

    bool MTRenderer::initialize(const Config& config)
    {
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;
        
        // make view support metal
        makeViewMetalCompatible( (void*)config.windowHandle);
        
        // set view port
        Viewport viewport(0, 0, m_screenWidth, m_screenHeight);
        setViewport(&viewport);
        
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
    
    Texture* MTRenderer::createTexture2D(const String& name)
    {
        return EchoNew(MTTexture2D);
    }
    
    void MTRenderer::makeViewMetalCompatible(void* handle)
    {
        NSView* view = (NSView*)handle;
        
        if (![view.layer isKindOfClass:[CAMetalLayer class]])
        {
            [view setLayer:[CAMetalLayer layer]];
            [view setWantsLayer:YES];
        }
    }
}
