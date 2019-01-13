#include "vk_renderer.h"
#include "vk_frame_buffer.h"
#include "vk_renderable.h"

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
    
    Renderable* VKRenderer::createRenderable(const String& renderStage, ShaderProgramRes* material)
    {
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;
        
        return renderable;
    }
}
