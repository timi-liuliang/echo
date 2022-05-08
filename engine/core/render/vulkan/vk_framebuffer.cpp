#include "engine/core/util/PathUtil.h"
#include "base/Renderer.h"
#include "vk_framebuffer.h"
#include "vk_renderer.h"
#include "vk_mapping.h"

namespace Echo
{
    VKFramebuffer::VKFramebuffer()
    {
    }

    VKFramebuffer::~VKFramebuffer()
    {
    }

    void VKFramebuffer::destroyVkRenderPass()
    {
        if (m_vkRenderPass)
        {
            vkDestroyRenderPass(VKRenderer::instance()->getVkDevice(), m_vkRenderPass, nullptr);
            m_vkRenderPass = VK_NULL_HANDLE;
        }
    }

    VKFramebufferOffscreen::VKFramebufferOffscreen(ui32 width, ui32 height)
        : FrameBufferOffScreen(width, height)
    {
    }

    VKFramebufferOffscreen::~VKFramebufferOffscreen()
    {
    }

    bool VKFramebufferOffscreen::begin()
    {
        return true;
    }

    bool VKFramebufferOffscreen::end()
    {
        return true;
    }

    void VKFramebufferOffscreen::onSize(ui32 width, ui32 height)
    {
        m_vkViewport.x = 0.f;
        m_vkViewport.y = 0.0f;
        m_vkViewport.width = width;
        m_vkViewport.height = height;
        m_vkViewport.minDepth = 0.f;
        m_vkViewport.maxDepth = 1.f;

        m_vkScissor.offset = { 0, 0 };
        m_vkScissor.extent = { width, height };

        m_vkViewportStateCreateInfo = {};
        m_vkViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_vkViewportStateCreateInfo.pNext = nullptr;
        m_vkViewportStateCreateInfo.flags = 0;
        m_vkViewportStateCreateInfo.viewportCount = 1;
        m_vkViewportStateCreateInfo.pViewports = &m_vkViewport;
        m_vkViewportStateCreateInfo.scissorCount = 1;
        m_vkViewportStateCreateInfo.pScissors = &m_vkScissor;
    }

    void VKFramebufferOffscreen::createVkFramebuffers()
    {
        //VKRenderView* colorView = ECHO_DOWN_CAST<VKRenderView*>(m_views[ui8(Attachment::Color0)]);
        //if (colorView)
        //{
        //    VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());
        //    VkImageView vkImageView = colorView->getVkImageView();

        //    VkFramebufferCreateInfo fbCreateInfo = {};
        //    fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        //    fbCreateInfo.renderPass = m_vkRenderPass;
        //    fbCreateInfo.attachmentCount = 1;
        //    fbCreateInfo.pAttachments = &vkImageView;
        //    fbCreateInfo.width = m_width;
        //    fbCreateInfo.height = m_height;
        //    fbCreateInfo.layers = 1;

        //    VKDebug(vkCreateFramebuffer(vkRenderer->getVkDevice(), &fbCreateInfo, NULL, &m_vkFramebuffers));
        //}
    }

    void VKFramebufferOffscreen::createVkRenderPass()
    {
        assert(false);
    }

    VKFramebufferWindow::VKFramebufferWindow()
        : FrameBufferWindow()
    {
        const Renderer::Settings& settings = Renderer::instance()->getSettings();

        createVkSemaphores();

        createVkSurface((void*)settings.m_windowHandle);

        vkGetDeviceQueue(VKRenderer::instance()->getVkDevice(), VKRenderer::instance()->getPresentQueueFamilyIndex(m_vkWindowSurface), 0, &m_vkPresentQueue);
    }

    VKFramebufferWindow::~VKFramebufferWindow()
    {
        destroyVkFramebuffers();
        destroyVkColorImageViews();
        destroyVkDepthImageView();
        destroyVkSwapChain(m_vkSwapChain);

        vkDestroySurfaceKHR(VKRenderer::instance()->getVkInstance(), m_vkWindowSurface, nullptr);
        vkDestroySemaphore(VKRenderer::instance()->getVkDevice(), m_vkRenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(VKRenderer::instance()->getVkDevice(), m_vkImageAvailableSemaphore, nullptr);
    }

    bool VKFramebufferWindow::begin()
    {
        VKDebug(vkAcquireNextImageKHR(VKRenderer::instance()->getVkDevice(), m_vkSwapChain, Math::MAX_UI64, m_vkImageAvailableSemaphore, VK_NULL_HANDLE, &m_imageIndex));

		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

		if (VK_SUCCESS == vkBeginCommandBuffer(getVkCommandbuffer(), &commandBufferBeginInfo))
		{
			array<VkClearValue, 2> clearValues;
			clearValues[0].color = { m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a };
			clearValues[1].depthStencil = { m_clearDepth, m_clearStencil };

			VkRenderPassBeginInfo renderPassBeginInfo = {};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.pNext = nullptr;
			renderPassBeginInfo.renderPass = m_vkRenderPass;
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = m_vkViewport.width;
			renderPassBeginInfo.renderArea.extent.height = m_vkViewport.height;
			renderPassBeginInfo.clearValueCount = clearValues.size();
			renderPassBeginInfo.pClearValues = clearValues.data();
			renderPassBeginInfo.framebuffer = getVkFramebuffer();

			vkCmdBeginRenderPass(getVkCommandbuffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			return true;
		}

		EchoLogError("vulkan begin command buffer failed.");
		return false;
    }

    bool VKFramebufferWindow::end()
    {
        submitCommandBuffer();

        present();

        return true;
    }

    void VKFramebufferWindow::onSize(ui32 width, ui32 height)
    {
		m_vkViewport.x = 0.f;
		m_vkViewport.y = 0.0f;
		m_vkViewport.width = width;
		m_vkViewport.height = height;
		m_vkViewport.minDepth = 0.f;
		m_vkViewport.maxDepth = 1.f;

        m_vkScissor.offset = { 0, 0 };
        m_vkScissor.extent = { width, height};

		m_vkViewportStateCreateInfo = {};
		m_vkViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        m_vkViewportStateCreateInfo.pNext = nullptr;
        m_vkViewportStateCreateInfo.flags = 0;
		m_vkViewportStateCreateInfo.viewportCount = 1;
		m_vkViewportStateCreateInfo.pViewports = &m_vkViewport;
        m_vkViewportStateCreateInfo.scissorCount = 1;
        m_vkViewportStateCreateInfo.pScissors = &m_vkScissor;

        recreateVkSwapChain();
    }

	// https://vulkan.lunarg.com/doc/view/1.2.162.1/mac/tutorial/html/10-init_render_pass.html
	void VKFramebufferWindow::createVkRenderPass()
	{
		destroyVkRenderPass();

		if (!m_vkRenderPass)
		{
			VkAttachmentReference colorRef = {};
			colorRef.attachment = 0;
			colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthRef = {};
			depthRef.attachment = 1;
			depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			// The pipelineBindPoint member is meant to indicate if this is a graphics or a compute subpass
			VkSubpassDescription subpassDesc = {};
			subpassDesc.flags = 0;
			subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDesc.inputAttachmentCount = 0;
			subpassDesc.pInputAttachments = nullptr;
			subpassDesc.colorAttachmentCount = 1;
			subpassDesc.pColorAttachments = &colorRef;
			subpassDesc.pResolveAttachments = nullptr;
			subpassDesc.pDepthStencilAttachment = &depthRef;
			subpassDesc.preserveAttachmentCount = 0;
			subpassDesc.pPreserveAttachments = nullptr;

			array<VkAttachmentDescription, 2> attachDescs;
			attachDescs[0].flags = 0;
			attachDescs[0].format = VKMapping::mapPixelFormat(m_colorFormat);
			attachDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
			attachDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachDescs[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			attachDescs[1].flags = 0;
            attachDescs[1].format = VKMapping::mapPixelFormat(m_vkDepthImageView->getPixelFormat());
			attachDescs[1].samples = VK_SAMPLE_COUNT_1_BIT;
			attachDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachDescs[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDependency subpassDependency = {};
			subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			subpassDependency.dstSubpass = 0;
			subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			subpassDependency.srcAccessMask = 0;
			subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			subpassDependency.dependencyFlags = 0;

			VkRenderPassCreateInfo renderPassCreateInfo = {};
			renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCreateInfo.pNext = nullptr;
			renderPassCreateInfo.flags = 0;
			renderPassCreateInfo.attachmentCount = attachDescs.size();
			renderPassCreateInfo.pAttachments = attachDescs.data();
			renderPassCreateInfo.subpassCount = 1;
			renderPassCreateInfo.pSubpasses = &subpassDesc;
			renderPassCreateInfo.dependencyCount = 1;
			renderPassCreateInfo.pDependencies = &subpassDependency;

			VKDebug(vkCreateRenderPass(VKRenderer::instance()->getVkDevice(), &renderPassCreateInfo, nullptr, &m_vkRenderPass));
		}
	}

    void VKFramebufferWindow::recreateVkSwapChain()
    {
        VkDevice vkDevice = VKRenderer::instance()->getVkDevice();
        if (vkDevice)
        {
            VKDebug(vkDeviceWaitIdle(vkDevice));

            createSwapChain(vkDevice);
            createVkColorImageViews(vkDevice);
            createVkDepthImageView();
            createVkRenderPass();
            createVkFramebuffers();
            createVkCommandBuffers();
            createVkFences();

            VKDebug(vkDeviceWaitIdle(vkDevice));
        }
    }

    void VKFramebufferWindow::destroyVkSwapChain(VkSwapchainKHR vkSwapChain)
    {
        if (vkSwapChain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(VKRenderer::instance()->getVkDevice(), vkSwapChain, nullptr);
    }

    void VKFramebufferWindow::createVkCommandBuffers()
    {
        destroyVkCommandBuffers();

        m_vkCommandBuffers.resize(m_vkSwapChainImages.size());
        for (size_t i = 0; i < m_vkCommandBuffers.size(); i++)
        {
            m_vkCommandBuffers[i] = VKRenderer::instance()->createVkCommandBuffer();
        }
    }

    void VKFramebufferWindow::destroyVkCommandBuffers()
    {
        if (m_vkCommandBuffers.empty())
            vkFreeCommandBuffers(VKRenderer::instance()->getVkDevice(), VKRenderer::instance()->getVkCommandPool(), m_vkCommandBuffers.size(), m_vkCommandBuffers.data());
    }

    void VKFramebufferWindow::createVkSemaphores()
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;

        VKDebug(vkCreateSemaphore(VKRenderer::instance()->getVkDevice(), &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphore));
        VKDebug(vkCreateSemaphore(VKRenderer::instance()->getVkDevice(), &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphore));
    }

    void VKFramebufferWindow::createVkFences()
    {
        destroyVkFences();

        // Fences (Used to check draw command buffer completion)
        VkFenceCreateInfo fenceCreateInfo = {};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;

        // Create in signaled state so we don't wait on first render of each command buffer
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        m_waitFences.resize(m_vkSwapChainImages.size());
        for (VkFence& fence : m_waitFences)
            VKDebug(vkCreateFence(VKRenderer::instance()->getVkDevice(), &fenceCreateInfo, nullptr, &fence));
    }

    void VKFramebufferWindow::destroyVkFences()
    {
        for (VkFence& fence : m_waitFences)
            vkDestroyFence(VKRenderer::instance()->getVkDevice(), fence, nullptr);

        m_waitFences.clear();
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
        //createInfo.window = AndroidGetApplicationWindow();
        //if (VK_SUCCESS != vkCreateAndroidSurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_vkWindowSurface))
        {
            EchoLogError("Vulkan Renderer failed to create window surface!");
        }
#endif

		// surface capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		if (VK_SUCCESS == vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VKRenderer::instance()->getVkPhysicalDevice(), m_vkWindowSurface, &surfaceCapabilities))
		{
			ui32 width = surfaceCapabilities.currentExtent.width;
			ui32 height = surfaceCapabilities.currentExtent.height;
			onSize(width, height);
		}
    }

    void VKFramebufferWindow::submitCommandBuffer()
    {
        vkCmdEndRenderPass(getVkCommandbuffer());

        // end command buffer before submit
        VKDebug(vkEndCommandBuffer(getVkCommandbuffer()));

        // Use a fence to wait until the command buffer has finished execution before using it again
        VKDebug(vkWaitForFences(VKRenderer::instance()->getVkDevice(), 1, &m_waitFences[m_imageIndex], VK_TRUE, UINT64_MAX));
        VKDebug(vkResetFences(VKRenderer::instance()->getVkDevice(), 1, &m_waitFences[m_imageIndex]));

		// wait stage flags
        VkSemaphore waitSemaphores[] = { m_vkImageAvailableSemaphore };
        VkSemaphore signalSemaphores[] = { m_vkRenderFinishedSemaphore };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_vkCommandBuffers[m_imageIndex];
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;							            // Semaphore(s) to wait upon before the submitted command buffer starts executing
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;						// Semaphore(s) to be signaled when command buffers have completed
        submitInfo.pWaitDstStageMask = waitStages;

        VKDebug(vkQueueSubmit(VKRenderer::instance()->getVkGraphicsQueue(), 1, &submitInfo, m_waitFences[m_imageIndex]));
    }

    void VKFramebufferWindow::present()
    {
        VkPresentInfoKHR present = {};
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.pNext = nullptr;
		present.waitSemaphoreCount = 1;
		present.pWaitSemaphores = &m_vkRenderFinishedSemaphore;
        present.swapchainCount = 1;
        present.pSwapchains = &m_vkSwapChain;
        present.pImageIndices = &m_imageIndex;
        present.pResults = nullptr;

        VKDebug(vkQueuePresentKHR(m_vkPresentQueue, &present));
        VKDebug(vkQueueWaitIdle(m_vkPresentQueue));
    }

    void VKFramebufferWindow::createSwapChain(VkDevice vkDevice)
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        // remember old swapchain
        VkSwapchainKHR oldVkSwapChain = m_vkSwapChain;

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
        createInfo.oldSwapchain = oldVkSwapChain;
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

        // destroy old vk swapchain and clearsup all the presentable images
        destroyVkColorImageViews();
        destroyVkDepthImageView();
        destroyVkSwapChain(oldVkSwapChain);
    }

    void VKFramebufferWindow::createVkColorImageViews(VkDevice vkDevice)
    {
        // destroyVkImageViews has been called by createSwapChain()
        // do nothing

        // image count
        ui32 swapChainImageCount = 0;
        VKDebug(vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, nullptr));

        // create image views
        if (swapChainImageCount)
        {
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
                createInfo.format = VKMapping::mapPixelFormat(m_colorFormat);
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
    }

    void VKFramebufferWindow::destroyVkColorImageViews()
    {
        for (VkImageView imageView : m_vkSwapChainImageViews)
            vkDestroyImageView(VKRenderer::instance()->getVkDevice(), imageView, nullptr);

        m_vkSwapChainImageViews.clear();
    }

    void VKFramebufferWindow::createVkDepthImageView()
    {
        m_vkDepthImageView = ECHO_DOWN_CAST<VKTextureRender*>(VKRenderer::instance()->createTextureRender("VK_WINDOW_DEPTH_IMAGE_VIEW"));
        m_vkDepthImageView->updateTexture2D(m_depthFormat, Texture::TU_GPU_READ, m_vkViewport.width, m_vkViewport.height, nullptr, 0);
    }

    void VKFramebufferWindow::destroyVkDepthImageView()
    {
        EchoSafeDelete(m_vkDepthImageView, VKTextureRender);
    }

    void VKFramebufferWindow::createVkFramebuffers()
    {
        destroyVkFramebuffers();

		// surface capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VKRenderer::instance()->getVkPhysicalDevice(), m_vkWindowSurface, &surfaceCapabilities);

        m_vkFramebuffers.resize(m_vkSwapChainImages.size());
        for (size_t i = 0; i < m_vkFramebuffers.size(); i++)
        {
            vector<VkImageView>::type attachments = { m_vkSwapChainImageViews[i], m_vkDepthImageView->getVkImageView() };

            VkFramebufferCreateInfo fbCreateInfo = {};
            fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            fbCreateInfo.pNext = nullptr;
            fbCreateInfo.flags = 0;
            fbCreateInfo.renderPass = m_vkRenderPass;
            fbCreateInfo.attachmentCount = attachments.size();
            fbCreateInfo.pAttachments = attachments.data();
            fbCreateInfo.width = surfaceCapabilities.currentExtent.width;
            fbCreateInfo.height = surfaceCapabilities.currentExtent.height;
            fbCreateInfo.layers = 1;

            VKDebug(vkCreateFramebuffer(VKRenderer::instance()->getVkDevice(), &fbCreateInfo, NULL, &m_vkFramebuffers[i]));
        }
    }

    void VKFramebufferWindow::destroyVkFramebuffers()
    {
        for (VkFramebuffer frameBuffer : m_vkFramebuffers)
            vkDestroyFramebuffer(VKRenderer::instance()->getVkDevice(), frameBuffer, nullptr);

        m_vkFramebuffers.clear();
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
