#pragma once

#include "vk_render_base.h"

namespace Echo
{
	class VKSwapChain
	{
	public:
		VKSwapChain();
		~VKSwapChain();

		// create
		void create(VkDevice vkDevice);

	private:
		// create swap chain
		void createSwapChain(VkDevice vkDevice);

		// create image views
		void createImageViews(VkDevice vkDevice);

	private:
		VkSwapchainKHR				m_vkSwapChain = VK_NULL_HANDLE;
		vector<VkImageView>::type	m_vkSwapChainImageViews;
	};
}
