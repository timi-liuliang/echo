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

    void VkFramebuffer::createVkRenderPass()
    {
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

        if(VK_SUCCESS != vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, nullptr, &m_vkRenderPass))
        {
            EchoLogError("vulkan create render pass failed.");
        }
    }

    void VkFramebuffer::createVkFramebuffer()
    {
        VKRenderView* colorView = ECHO_DOWN_CAST<VKRenderView*>(m_views[Attachment::Color0]);
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
}
