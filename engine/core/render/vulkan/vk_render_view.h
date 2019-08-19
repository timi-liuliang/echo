#pragma once

#include "interface/RenderView.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKRenderView : public RenderView
	{
	public:
        VKRenderView(ui32 width, ui32 height, PixelFormat pixelFormat);
		virtual ~VKRenderView();

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;

    private:
        VkFormat        m_vkFormat;
        VkImage         m_vkImage;
        VkDeviceMemory  m_vkMemory;
        VkImageView     m_vkImageView;
	};
}
