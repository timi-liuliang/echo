##include "vk_swap_chain.h"

namespace Echo
{
	VKSwapChain::VKSwapChain()
	{

	}

	VKSwapChain::~VKSwapChain()
	{

	}

	void VKSwapChain::create()
	{
		createSwapChain();

		createImageViews();
	}

	void VKSwapChain::createSwapChain()
	{
		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.surface = surface;
		createInfo.minImageCount = desiredNumberOfSwapchainImages;
		createInfo.imageFormat = colorFormat;
		createInfo.imageColorSpace = colorSpace;
		createInfo.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		createInfo.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		createInfo.imageArrayLayers = 1;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = NULL;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.oldSwapchain = oldSwapchain;
		createInfo.clipped = VK_TRUE;
		createInfo.compositeAlpha = compositeAlpha;

		if (vkCreateSwapchainKHR(m_vkDevice, &createInfo, nullptr, &m_vkSwapChain) != VK_SUCCESS)
		{
			EchoLogError("Failed to create vulkan swap chain!");
		}
	}

	void VKSwapChain::createImageViews()
	{
		m_vkSwapChainImageViews.resize(swapChainImages.size());

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &m_vkSwapChainImageViews[i]) != VK_SUCCESS)
		{
		    EchoLogError("Failed to create image views!");
		}
	}
}
