#include "vk_framebuffer_window.h"
#include "vk_renderer.h"

namespace Echo
{
    VKFramebufferWindow::VKFramebufferWindow(ui32 width, ui32 height)
        : FrameBuffer(0, width, height)
    {
    }

    VKFramebufferWindow::~VKFramebufferWindow()
    {
    }

    bool VKFramebufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
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