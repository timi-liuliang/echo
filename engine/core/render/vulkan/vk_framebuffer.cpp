#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "vk_framebuffer.h"

namespace Echo
{
    VKFramebuffer::VKFramebuffer(ui32 id, ui32 width, ui32 height)
        : FrameBuffer(id, width, height)
    {
    }

    VKFramebuffer::~VKFramebuffer()
    {
    }

    void VKFramebuffer::attach(Attachment attachment, RenderView* renderView)
    {
        m_views[(ui8)attachment] = renderView;
    }

    bool VKFramebuffer::begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue)
    {
        Renderer::instance()->setDepthStencilState(Renderer::instance()->getDefaultDepthStencilState());

        return true;
    }

    bool VKFramebuffer::end()
    {
        return true;
    }

    void VKFramebuffer::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;

        for (RenderView* colorView : m_views)
        {
            if (colorView)
                colorView->onSize(width, height);
        }
    }
}
