#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "vk_render_view.h"
#include "vk_framebuffer.h"
#include "vk_renderer.h"

namespace Echo
{
    static VKFramebuffer* g_current = nullptr;

    VKFramebuffer::VKFramebuffer(ui32 id, ui32 width, ui32 height)
        : FrameBuffer(id, width, height)
    {
        createVkCommandBuffer();
        createVkRenderPass();
        createVkFramebuffer();
        createVkDescriptorPool();
    }

    VKFramebuffer::~VKFramebuffer()
    {
        vkDestroyDescriptorPool(VKRenderer::instance()->getVkDevice(), m_vkDescriptorPool, nullptr);
    }

    VKFramebuffer* VKFramebuffer::current()
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

        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        //commandBufferBeginInfo.pNext = nullptr;
        //commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        //commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (VK_SUCCESS == vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo))
        {
            VkClearValue clearValues[2];
            clearValues[0].color = { { 1.0f, 0.0f, 0.2f, 1.0f } }; // { Renderer::BGCOLOR.r, Renderer::BGCOLOR.g, Renderer::BGCOLOR.b, Renderer::BGCOLOR.a };
            clearValues[1].depthStencil = { 1.0f, 0 };

            VkRenderPassBeginInfo renderPassBeginInfo = {};
            renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassBeginInfo.pNext = nullptr;
            renderPassBeginInfo.renderPass = m_vkRenderPass;
            renderPassBeginInfo.renderArea.offset.x = 0;
            renderPassBeginInfo.renderArea.offset.y = 0;
            renderPassBeginInfo.renderArea.extent.width = m_width;
            renderPassBeginInfo.renderArea.extent.height = m_height;
            renderPassBeginInfo.clearValueCount = 1;
            renderPassBeginInfo.pClearValues = clearValues;
            renderPassBeginInfo.framebuffer = m_vkFramebuffer;

            vkCmdBeginRenderPass(m_vkCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

            return true;
        }

        EchoLogError("vulkan begin command buffer failed.");
        return false;
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

        m_vkViewportStateCreateInfo = {};
        m_vkViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_vkViewportStateCreateInfo.viewportCount = 1;
        m_vkViewportStateCreateInfo.pViewports = &m_vkViewport;
        //m_vkViewportStateCreateInfo.scissorCount = 1;
        //m_vkViewportStateCreateInfo.pScissors = nullptr;
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
        //attachDesc.format = ;
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

        VKDebug(vkCreateRenderPass(vkRenderer->getVkDevice(), &renderPassCreateInfo, nullptr, &m_vkRenderPass));
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

            VKDebug(vkCreateFramebuffer(vkRenderer->getVkDevice(), &fbCreateInfo, NULL, &m_vkFramebuffer));
        }
    }

    void VKFramebuffer::createVkCommandBuffer()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        VkCommandBufferAllocateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.commandPool = vkRenderer->getVkCommandPool();
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.commandBufferCount = 1;

        VKDebug(vkAllocateCommandBuffers(vkRenderer->getVkDevice(), &createInfo, &m_vkCommandBuffer));
    }

    void VKFramebuffer::createVkDescriptorPool()
    {
        array<VkDescriptorPoolSize, 1> typeCounts;
        typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        typeCounts[0].descriptorCount = 512;

        // For additional type you need to add new entries in the type count list
        //typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        //typeCounts[1].descriptorCount = 2;

        // Create the global descriptor pool
        VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
        descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolInfo.pNext = nullptr;
        descriptorPoolInfo.poolSizeCount = typeCounts.size();
        descriptorPoolInfo.pPoolSizes = typeCounts.data();
        descriptorPoolInfo.maxSets = 512;

        VKDebug(vkCreateDescriptorPool(VKRenderer::instance()->getVkDevice(), &descriptorPoolInfo, nullptr, &m_vkDescriptorPool));
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
        VKFramebuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);

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

    VKFramebufferWindow::VKFramebufferWindow(ui32 width, ui32 height, void* handle)
        : VKFramebuffer(0, width, height)
    {
        createVkSemaphores();

        createVkFences();

        createVkSurface(handle);

        createSwapChain(VKRenderer::instance()->getVkDevice());

        createImageViews(VKRenderer::instance()->getVkDevice());

        createVkWindowFramebuffer();

        vkGetDeviceQueue(VKRenderer::instance()->getVkDevice(), VKRenderer::instance()->getPresentQueueFamilyIndex(m_vkWindowSurface), 0, &m_vkPresentQueue);

        onSize(width, height);

        g_current = this;
    }

    VKFramebufferWindow::~VKFramebufferWindow()
    {
        vkDestroySurfaceKHR(VKRenderer::instance()->getVkInstance(), m_vkWindowSurface, nullptr);
        vkDestroySemaphore(VKRenderer::instance()->getVkDevice(), m_vkRenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(VKRenderer::instance()->getVkDevice(), m_vkImageAvailableSemaphore, nullptr);
    }

    bool VKFramebufferWindow::begin(bool clearColor, const Color& backgroundColor, bool clearDepth, float depthValue, bool clearStencil, ui8 stencilValue)
    {
        VKDebug(vkAcquireNextImageKHR(VKRenderer::instance()->getVkDevice(), m_vkSwapChain, Math::MAX_UI64, m_vkImageAvailableSemaphore, VK_NULL_HANDLE, &m_imageIndex));

        return VKFramebuffer::begin(clearColor, backgroundColor, clearDepth, depthValue, clearStencil, stencilValue);
    }

    bool VKFramebufferWindow::end()
    {
        submitCommandBuffer();

        present();

        return true;
    }

    void VKFramebufferWindow::onSize(ui32 width, ui32 height)
    {
        VKFramebuffer::onSize(width, height);
    }

    void VKFramebufferWindow::createVkSemaphores()
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VKDebug(vkCreateSemaphore(VKRenderer::instance()->getVkDevice(), &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphore));
        VKDebug(vkCreateSemaphore(VKRenderer::instance()->getVkDevice(), &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphore));
    }

    void VKFramebufferWindow::createVkFences()
    {
        // Fences (Used to check draw command buffer completion)
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        // Create in signaled state so we don't wait on first render of each command buffer
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_waitFences.resize(1);
        for (VkFence& fence : m_waitFences)
            VKDebug(vkCreateFence(VKRenderer::instance()->getVkDevice(), &fenceCreateInfo, nullptr, &fence));
    }

    void VKFramebufferWindow::createVkSurface(void* handle)
    {
        // Create window surface
#ifdef ECHO_PLATFORM_WINDOWS
        VkWin32SurfaceCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.hwnd = (HWND)handle;
        createInfo.hinstance = GetModuleHandle(nullptr);

        // create surface
        VKDebug(vkCreateWin32SurfaceKHR(VKRenderer::instance()->getVkInstance(), &createInfo, nullptr, &m_vkWindowSurface));

#elif defined(ECHO_PLATFORM_ANDROID)
        VkAndroidSurfaceCreateInfoKHR createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.flags = 0;
        createInfo.window = AndroidGetApplicationWindow();
        if (VK_SUCCESS != vkCreateAndroidSurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_vkWindowSurface))
        {
            EchoLogError("Vulkan Renderer failed to create window surface!");
        }
#endif
    }

    void VKFramebufferWindow::submitCommandBuffer()
    {
        vkCmdEndRenderPass(m_vkCommandBuffer);

        // end command buffer before submit
        vkEndCommandBuffer(m_vkCommandBuffer);

        // Use a fence to wait until the command buffer has finished execution before using it again
        VKDebug(vkWaitForFences(VKRenderer::instance()->getVkDevice(), 1, &m_waitFences[m_imageIndex], VK_TRUE, UINT64_MAX));
        VKDebug(vkResetFences(VKRenderer::instance()->getVkDevice(), 1, &m_waitFences[m_imageIndex]));

        // wait stage flags
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_vkCommandBuffer;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_vkImageAvailableSemaphore;							// Semaphore(s) to wait upon before the submitted command buffer starts executing
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_vkRenderFinishedSemaphore;						// Semaphore(s) to be signaled when command buffers have completed
        submitInfo.pWaitDstStageMask = waitStages;

        VKDebug(vkQueueSubmit(VKRenderer::instance()->getVkGraphicsQueue(), 1, &submitInfo, m_waitFences[m_imageIndex]));
    }

    void VKFramebufferWindow::present()
    {
        VkPresentInfoKHR present = {};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.pNext = nullptr;
        present.swapchainCount = 1;
        present.pSwapchains = &m_vkSwapChain;
        present.pImageIndices = &m_imageIndex;
        present.pWaitSemaphores = &m_vkRenderFinishedSemaphore;
        present.waitSemaphoreCount = 1;
        present.pResults = nullptr;

        VKDebug(vkQueuePresentKHR(m_vkPresentQueue, &present));
        VKDebug(vkQueueWaitIdle(m_vkPresentQueue));
    }

    void VKFramebufferWindow::createSwapChain(VkDevice vkDevice)
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        // surface capabilities
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkRenderer->getVkPhysicalDevice(), m_vkWindowSurface, &surfaceCapabilities);

        // present mode
        ui32 presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkRenderer->getVkPhysicalDevice(), m_vkWindowSurface, &presentModeCount, nullptr);

        vector<VkPresentModeKHR>::type presentModes(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(vkRenderer->getVkPhysicalDevice(), m_vkWindowSurface, &presentModeCount, &presentModes[0]);

        // swap chain extent
        VkExtent2D swapChainExtent = surfaceCapabilities.currentExtent;

        // surface format
        VkSurfaceFormatKHR surfaceFormat = pickSurfaceSupportFormat();

        // The FIFO present mode is guaranteed by the spec to be supported
        // Also note that current Android driver only supports FIFO
        VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

        // Detemine the number of VkImage's to use in the swap chain.
        ui32 desiredNumberOfSwapChainImages = surfaceCapabilities.minImageCount;

        // surface transform bit
        VkSurfaceTransformFlagBitsKHR presentTransform = surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : surfaceCapabilities.currentTransform;

        // Find a supported composite alpha mode 
        VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        // Queue family indices
        ui32 queueFamilyIndices[2] = { vkRenderer->getGraphicsQueueFamilyIndex(), vkRenderer->getPresentQueueFamilyIndex(m_vkWindowSurface) };

        // Swap chain info
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext = nullptr;
        createInfo.surface = m_vkWindowSurface;
        createInfo.minImageCount = desiredNumberOfSwapChainImages;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = { swapChainExtent.width, swapChainExtent.height };
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.preTransform = presentTransform;
        createInfo.imageArrayLayers = 1;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.presentMode = swapChainPresentMode;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        createInfo.clipped = VK_TRUE;
        createInfo.compositeAlpha = compositeAlpha;

        // queue family index config
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        if (queueFamilyIndices[0] != queueFamilyIndices[1])
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }

        VKDebug(vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &m_vkSwapChain));
    }

    void VKFramebufferWindow::createImageViews(VkDevice vkDevice)
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        // image count
        ui32 swapChainImageCount = 0;
        VKDebug(vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, nullptr));

        // Vk image
        m_vkSwapChainImages.resize(swapChainImageCount);
        VKDebug(vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, &m_vkSwapChainImages[0]));

        // create ImageViews
        m_vkSwapChainImageViews.resize(swapChainImageCount);
        for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++)
        {
            VkImageViewCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = m_vkSwapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            VKDebug(vkCreateImageView(vkDevice, &createInfo, nullptr, &m_vkSwapChainImageViews[i]));
        }
    }

    void VKFramebufferWindow::createVkWindowFramebuffer()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());
        VkImageView vkImageView = m_vkSwapChainImageViews[ui8(Attachment::Color0)];

        VkFramebufferCreateInfo fbCreateInfo = {};
        fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbCreateInfo.renderPass = m_vkRenderPass;
        fbCreateInfo.attachmentCount = 1;
        fbCreateInfo.pAttachments = &vkImageView;
        fbCreateInfo.width = m_width;
        fbCreateInfo.height = m_height;
        fbCreateInfo.layers = 1;

        VKDebug(vkCreateFramebuffer(vkRenderer->getVkDevice(), &fbCreateInfo, NULL, &m_vkFramebuffer));
    }

    VkSurfaceFormatKHR VKFramebufferWindow::pickSurfaceSupportFormat()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        ui32 formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkRenderer->getVkPhysicalDevice(), m_vkWindowSurface, &formatCount, nullptr);

        vector<VkSurfaceFormatKHR>::type surfaceFormats(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vkRenderer->getVkPhysicalDevice(), m_vkWindowSurface, &formatCount, &surfaceFormats[0]);

        return surfaceFormats[0];
    }
}
