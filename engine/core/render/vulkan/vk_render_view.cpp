#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "vk_render_view.h"
#include "vk_renderer.h"

namespace Echo
{
    VKRenderView::VKRenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
		: RenderView(width, height, pixelFormat)
	{
        mappingVkFormat();

        // Image views enable us to access the image using a format which is
        // different than the native image format. For example, if the underlying
        // format is 16 bit we can access it as a single 16 bit channel or two 8
        // bits channel. There are many restrictions about valid combinations.
        createVkImage();
        createVkImageView();
	}

    VKRenderView::~VKRenderView()
	{

	}

	void VKRenderView::onSize( ui32 _width, ui32 _height )
	{
	}

    void VKRenderView::mappingVkFormat()
    {
    #ifdef ECHO_PLATFORM_ANDROID
        m_vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
    #else
        m_vkFormat = VK_FORMAT_D16_UNORM; // VK_FORMAT_D32_SFLOAT
    #endif
    }

    void VKRenderView::createVkImage()
    {
        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_vkFormat;
        imageInfo.extent.width = m_width;
        imageInfo.extent.height = m_height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        //imageInfo.samples = 0;// NUM_SAMPLES;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.queueFamilyIndexCount = 0;
        imageInfo.pQueueFamilyIndices = nullptr;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.flags = 0;

        VKDebug(vkCreateImage(VKRenderer::instance()->getVkDevice(), &imageInfo, nullptr, &m_vkImage));
    }

    void VKRenderView::createVkImageView()
    {
        if (m_vkImage)
        {
            VkImageViewCreateInfo imageViewCreateInfo = {};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = nullptr;
            imageViewCreateInfo.image = m_vkImage;
            imageViewCreateInfo.format = m_vkFormat;
            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
            imageViewCreateInfo.subresourceRange.aspectMask = PixelUtil::IsDepth(m_format) ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;
            imageViewCreateInfo.flags = 0;

            if (PixelUtil::IsStencil(m_format))
                imageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

            VKDebug(vkCreateImageView(VKRenderer::instance()->getVkDevice(), &imageViewCreateInfo, nullptr, &m_vkImageView));
        }
    }
}
