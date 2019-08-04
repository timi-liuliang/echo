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
		void create();

	private:
		// create swap chain
		void createSwapChain();

		// create image views
		void createImageViews();

	private:
		VkSwapchainKHR				m_vkSwapChain = VK_NULL_HANDLE;
		vector<VkImageView>::type	m_vkSwapChainImageViews;
	};
}
