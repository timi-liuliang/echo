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
		createInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
		createInfo.imageExtent = { swapChainExtent.width, swapChainExtent.height };
		createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = presentTransform;
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.presentMode = swapChainPresentMode;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		createInfo.clipped = VK_TRUE;
		createInfo.compositeAlpha = compositeAlpha;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = &queueFamilyIndices[0];

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
		vector<VkImage>::type swapChainImages(swapChainImageCount);
		vkGetSwapchainImagesKHR(vkDevice, m_vkSwapChain, &swapChainImageCount, &swapChainImages[0]);

		// create ImageViews
		m_vkSwapChainImageViews.resize(swapChainImageCount);
		for (size_t i = 0; i < m_vkSwapChainImageViews.size(); i++)
		{
			VkImageViewCreateInfo createInfo = {};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
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
}
