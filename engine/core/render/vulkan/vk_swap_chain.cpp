#include "vk_swap_chain.h"
#include "vk_renderer.h"

namespace Echo
{
	VKSwapChain::VKSwapChain()
	{

	}

	VKSwapChain::~VKSwapChain()
	{

	}

	void VKSwapChain::create(VkDevice vkDevice)
	{
		createSwapChain(vkDevice);

		createImageViews(vkDevice);
	}

	void VKSwapChain::createSwapChain(VkDevice vkDevice)
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		// surface capabilities
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkRenderer->getVkPhysicalDevice(), vkRenderer->getVkSurface(), &surfaceCapabilities);

		// present mode
		ui32 presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(vkRenderer->getVkPhysicalDevice(), vkRenderer->getVkSurface(), &presentModeCount, nullptr);

		vector<VkPresentModeKHR>::type presentModes(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vkRenderer->getVkPhysicalDevice(), vkRenderer->getVkSurface(), &presentModeCount, &presentModes[0]);

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
		ui32 queueFamilyIndices[2] = { vkRenderer->getGraphicsQueueFamilyIndex(), vkRenderer->getPresentQueueFamilyIndex() };

		// Swap chain info
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.surface = vkRenderer->getVkSurface();
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

		if (vkCreateSwapchainKHR(vkDevice, &createInfo, nullptr, &m_vkSwapChain) != VK_SUCCESS)
		{
			EchoLogError("Failed to create vulkan swap chain!");
		}
	}

	void VKSwapChain::createImageViews(VkDevice vkDevice)
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		// image count
		ui32 swapChainImageCount = 0;
		vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, nullptr);

		// Vk image
        m_vkSwapChainImages.resize(swapChainImageCount);
		vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, &m_vkSwapChainImages[0]);

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

			if (vkCreateImageView(vkDevice, &createInfo, nullptr, &m_vkSwapChainImageViews[i]) != VK_SUCCESS)
			{
				EchoLogError("Failed to create image views!");
			}
		}
	}

	VkSurfaceFormatKHR VKSwapChain::pickSurfaceSupportFormat()
	{
		VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

		ui32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkRenderer->getVkPhysicalDevice(), vkRenderer->getVkSurface(), &formatCount, nullptr);

		vector<VkSurfaceFormatKHR>::type surfaceFormats(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vkRenderer->getVkPhysicalDevice(), vkRenderer->getVkSurface(), &formatCount, &surfaceFormats[0]);

		return surfaceFormats[0];
	}
}
