#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "vk_render_view.h"
#include "vk_framebuffer.h"
#include "vk_renderer.h"

namespace Echo
{
    static VkFramebuffer* g_current = nullptr;

    VKFramebuffer::VKFramebuffer(ui32 id, ui32 width, ui32 height)
        : FrameBuffer(id, width, height)
    {
    }

    VKFramebuffer::~VKFramebuffer()
    {
    }

    VkFramebuffer* VkFramebuffer::current()
    {
        return g_current;
    }

    void VKFramebuffer::attach(Attachment attachment, RenderView* renderView)
    {
        m_views[(ui8)attachment] = renderView;
    }

    bool VKFramebuffer::begin(bool isClearColor, const Color& bgColor, bool isClearDepth, float depthValue, bool isClearStencil, ui8 stencilValue)
    {
        g_current = this;

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

        // view port
        m_vkViewport.x = 0.f;
        m_vkViewport.y = 0.0f;
        m_vkViewport.width = m_width;
        m_vkViewport.height = m_height;
        m_vkViewport.minDepth = 0.f;
        m_vkViewport.maxDepth = 1.f;
    }

    VkPipelineViewportStateCreateInfo VKFramebuffer::getVkViewportStateCreateInfo()
    {
        VkPipelineViewportStateCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        createInfo.viewportCount = 1;
        createInfo.pViewports = &m_vkViewport;

        return createInfo;
    }

    void VKFramebuffer::createVkRenderPass()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        VkAttachmentReference attachRef = {};
        attachRef.attachment = 0;
        attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &attachRef;

        VkAttachmentDescription attachDesc = {};
        //attachDesc.format = m_core.GetSurfaceFormat().format;
        attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachDesc;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;

        if(VK_SUCCESS != vkCreateRenderPass(vkRenderer->getVkDevice(), &renderPassCreateInfo, nullptr, &m_vkRenderPass))
        {
            EchoLogError("vulkan create render pass failed.");
        }
    }

    void VKFramebuffer::createVkFramebuffer()
    {
        VKRenderView* colorView = ECHO_DOWN_CAST<VKRenderView*>(m_views[ui8(Attachment::Color0)]);
        if(colorView)
        {
            VKRenderer* vkRenderer  = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());
            VkImageView vkImageView = colorView->getVkImageView();

            VkFramebufferCreateInfo fbCreateInfo = {};
            fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbCreateInfo.renderPass = m_vkRenderPass;
            fbCreateInfo.attachmentCount = 1;
            fbCreateInfo.pAttachments = &vkImageView;
            fbCreateInfo.width = m_width;
            fbCreateInfo.height = m_height;
            fbCreateInfo.layers = 1;

            if(VK_SUCCESS != vkCreateFramebuffer(vkRenderer->getVkDevice(), &fbCreateInfo, NULL, &m_vkFramebuffer))
            {
                EchoLogError("vulkan create frame buffer failed");
            }
        }
    }

    VKFramebufferOffscreen::VKFramebufferOffscreen(ui32 id, ui32 width, ui32 height)
        : VKFramebuffer(id, width, height)
    {
    }

    VKFramebufferOffscreen::~VKFramebufferOffscreen()
    {
    }

    bool VKFramebufferOffscreen::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        VkFramebuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);

        return true;
    }

    bool VKFramebufferOffscreen::end()
    {
        return true;
    }

    void VKFramebufferOffscreen::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
    }

    VKFramebufferWindow::VKFramebufferWindow(ui32 width, ui32 height)
        : VKFramebuffer(0, width, height)
    {
    }

    VKFramebufferWindow::~VKFramebufferWindow()
    {
    }

    bool VKFramebufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        VkFramebuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);

        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        ui32 imageIndex;
        vkAcquireNextImageKHR(vkRenderer->getVkDevice(), *vkRenderer->getVkSwapChain(), Math::MAX_UI64, vkRenderer->getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);

        VkCommandBuffer clearCommandBuffer = vkRenderer->getVkCommandBuffer();

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &clearCommandBuffer;

        if (VK_SUCCESS != vkQueueSubmit(vkRenderer->getVkGraphicsQueue(), 1, &submitInfo, nullptr))
        {
            EchoLogError("vulkan queue submit failed");
        }

        return true;
    }

    bool VKFramebufferWindow::end()
    {
        return true;
    }

    void VKFramebufferWindow::onSize(ui32 width, ui32 height)
    {
        m_width = width;
        m_height = height;
    }
}
