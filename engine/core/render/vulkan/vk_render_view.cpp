#include "engine/core/Util/PathUtil.h"
#include "interface/Renderer.h"
#include "vk_render_view.h"

namespace Echo
{
    VKRenderView::VKRenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
		: RenderView(width, height, pixelFormat)
	{
#ifdef ECHO_PLATFORM_ANDROID
        m_vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
#else
        m_vkFormat = VK_FORMAT_D16_UNORM; // VK_FORMAT_D32_SFLOAT
#endif

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_vkFormat;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
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

        VkMemoryAllocateInfo memAlloc = {};
        memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        memAlloc.pNext = nullptr;
        memAlloc.allocationSize = 0;
        memAlloc.memoryTypeIndex = 0;

        //VkImageViewCreateInfo view_info = {};
        //view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //view_info.pNext = NULL;
        //view_info.image = VK_NULL_HANDLE;
        //view_info.format = depth_format;
        //view_info.components.r = VK_COMPONENT_SWIZZLE_R;
        //view_info.components.g = VK_COMPONENT_SWIZZLE_G;
        //view_info.components.b = VK_COMPONENT_SWIZZLE_B;
        //view_info.components.a = VK_COMPONENT_SWIZZLE_A;
        //view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        //view_info.subresourceRange.baseMipLevel = 0;
        //view_info.subresourceRange.levelCount = 1;
        //view_info.subresourceRange.baseArrayLayer = 0;
        //view_info.subresourceRange.layerCount = 1;
        //view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        //view_info.flags = 0;

        //if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
        //    depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
        //    view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        //}

        //VkMemoryRequirements mem_reqs;

        ///* Create image */
        //res = vkCreateImage(info.device, &image_info, NULL, &info.depth.image);
        //assert(res == VK_SUCCESS);

        //vkGetImageMemoryRequirements(info.device, info.depth.image, &mem_reqs);

        //mem_alloc.allocationSize = mem_reqs.size;
        ///* Use the memory properties to determine the type of memory required */
        //pass =
        //    memory_type_from_properties(info, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex);
        //assert(pass);

        ///* Allocate memory */
        //res = vkAllocateMemory(info.device, &mem_alloc, NULL, &info.depth.mem);
        //assert(res == VK_SUCCESS);

        ///* Bind memory */
        //res = vkBindImageMemory(info.device, info.depth.image, info.depth.mem, 0);
        //assert(res == VK_SUCCESS);

        ///* Create image view */
        //view_info.image = info.depth.image;
        //res = vkCreateImageView(info.device, &view_info, NULL, &info.depth.view);
        //assert(res == VK_SUCCESS);
	}

    VKRenderView::~VKRenderView()
	{

	}

	void VKRenderView::onSize( ui32 _width, ui32 _height )
	{
	}
}
