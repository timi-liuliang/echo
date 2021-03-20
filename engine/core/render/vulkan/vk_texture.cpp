#include "vk_texture.h"
#include "vk_mapping.h"
#include "vk_renderer.h"
#include "engine/core/io/io.h"
#include "base/image/image.h"

namespace Echo
{
	VKTexture::VKTexture()
	{

	}

	VKTexture::~VKTexture()
	{

	}

	bool VKTexture::createVkImage(SamplerStatePtr samplerState, PixelFormat format, i32 width, i32 height, i32 depth, VkImageUsageFlags usage, VkFlags requirementsMask, VkImageTiling tiling, VkImageLayout initialLayout)
	{
		destroyVkImage();

		VkImageCreateInfo imageCreateInfo;
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.pNext = nullptr;
		imageCreateInfo.flags = 0;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = VKMapping::mapPixelFormat(format);
		imageCreateInfo.extent = { uint32_t(width), uint32_t(height), uint32_t(depth) };
		imageCreateInfo.mipLevels = 1;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = tiling;
		imageCreateInfo.usage = usage;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.queueFamilyIndexCount = 0;
		imageCreateInfo.pQueueFamilyIndices = nullptr;
		imageCreateInfo.initialLayout = initialLayout;

		VKDebug(vkCreateImage(VKRenderer::instance()->getVkDevice(), &imageCreateInfo, nullptr, &m_vkImage));
		if (m_vkImage)
		{
			createVkImageMemory(requirementsMask);
			createVkImageView(format);

			createDescriptorImageInfo(samplerState);

			return true;
		}

		return false;
	}

	void VKTexture::destroyVkImage()
	{
		if (m_vkImage)
		{
			vkDestroyImage(VKRenderer::instance()->getVkDevice(), m_vkImage, nullptr);
			m_vkImage = VK_NULL_HANDLE;
		}
	}

	void VKTexture::createVkImageMemory(VkFlags requirementsMask)
	{
		destroyVkImageMemory();

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(VKRenderer::instance()->getVkDevice(), m_vkImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo;
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = VKRenderer::instance()->findVkMemoryType(memRequirements.memoryTypeBits, requirementsMask);

		VKDebug(vkAllocateMemory(VKRenderer::instance()->getVkDevice(), &allocInfo, nullptr, &m_vkImageMemory));
		VKDebug(vkBindImageMemory(VKRenderer::instance()->getVkDevice(), m_vkImage, m_vkImageMemory, 0));
	}

	void VKTexture::destroyVkImageMemory()
	{
		if (m_vkImageMemory)
		{
			vkFreeMemory(VKRenderer::instance()->getVkDevice(), m_vkImageMemory, nullptr);
			m_vkImageMemory = VK_NULL_HANDLE;
		}
	}

	void VKTexture::createVkImageView(PixelFormat format)
	{
		destroyVkImageView();

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.image = m_vkImage;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = VKMapping::mapPixelFormat(format);
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = PixelUtil::IsDepth(format) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VKDebug(vkCreateImageView(VKRenderer::instance()->getVkDevice(), &createInfo, nullptr, &m_vkImageView));
	}

	void VKTexture::destroyVkImageView()
	{
		if (m_vkImageView)
		{
			vkDestroyImageView(VKRenderer::instance()->getVkDevice(), m_vkImageView, nullptr);
			m_vkImageView = VK_NULL_HANDLE;
		}
	}

	void VKTexture::createDescriptorImageInfo(SamplerStatePtr sampleState)
	{
		VKSamplerState* vkSamplerState = ECHO_DOWN_CAST<VKSamplerState*>(sampleState.ptr());

		m_vkDescriptorImageInfo.sampler = vkSamplerState->getVkSampler();
		m_vkDescriptorImageInfo.imageView = m_vkImageView;
		m_vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// https://github.com/SaschaWillems/Vulkan/blob/master/examples/texture/texture.cpp
	void VKTexture::setVkImageSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff, bool isUseStaging)
	{
		if (isUseStaging)
		{

		}
		else
		{
			void* data = nullptr;
			if (VK_SUCCESS == vkMapMemory(VKRenderer::instance()->getVkDevice(), m_vkImageMemory, 0, buff.getSize(), 0, &data))
			{
				memcpy(data, buff.getData(), buff.getSize());
				vkUnmapMemory(VKRenderer::instance()->getVkDevice(), m_vkImageMemory);

				// setup image memory barrier transfer image to shader read layout
				VkCommandBuffer copyCmd = VKRenderer::instance()->createVkCommandBuffer();

				// the sub resource range describes the regions of the image we will be transition
				VkImageSubresourceRange subResourcesRange = {};
				subResourcesRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				subResourcesRange.baseMipLevel = 0;
				subResourcesRange.levelCount = 1;
				subResourcesRange.layerCount = 1;

				// transition the texture image layout to shader read, so it can be sampled from
				VkImageMemoryBarrier imageMemoryBarrier = {};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.pNext = nullptr;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.srcQueueFamilyIndex = VKRenderer::instance()->getGraphicsQueueFamilyIndex();
				imageMemoryBarrier.dstQueueFamilyIndex = VKRenderer::instance()->getGraphicsQueueFamilyIndex();
				imageMemoryBarrier.image = m_vkImage;
				imageMemoryBarrier.subresourceRange = subResourcesRange;

				VkCommandBufferBeginInfo commandBufferBeginInfo = {};
				commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				commandBufferBeginInfo.pNext = nullptr;
				commandBufferBeginInfo.flags = 0;
				commandBufferBeginInfo.pInheritanceInfo = nullptr;

				if (VK_SUCCESS == vkBeginCommandBuffer(copyCmd, &commandBufferBeginInfo))
				{
					// insert a memory dependency at the proper pipeline stages that will execute the image layout transition
					// source pipeline stage is host write/read execution (VK_PIPELINE_STAGE_HOST_BIT)
					// destination pipeline stage fragment shader access (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT)
					vkCmdPipelineBarrier(
						copyCmd,
						VK_PIPELINE_STAGE_HOST_BIT,
						VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
						0,
						0, nullptr,
						0, nullptr,
						1, &imageMemoryBarrier);

					VKDebug(vkEndCommandBuffer(copyCmd));
				}

				VKRenderer::instance()->flushVkCommandBuffer(copyCmd, VKRenderer::instance()->getVkGraphicsQueue(), true);
			}
			else
			{
				EchoLogError("Vulkan vkMapMemory failed");
			}
		}
	}

	VKTexture2D::VKTexture2D(const String& pathName)
		: Texture(pathName)
	{

	}

    VKTexture2D::~VKTexture2D()
    {

    }

	void VKTexture2D::convertFormat(Image* image)
	{
		PixelFormat format = image->getPixelFormat();
		if (format == PixelFormat::PF_RGB8_SNORM)        image->convertFormat(PixelFormat::PF_RGBA8_SNORM);
		else if (format == PixelFormat::PF_RGB8_UNORM)   image->convertFormat(PixelFormat::PF_RGBA8_UNORM);
		else if (format == PixelFormat::PF_RGB8_UINT)    image->convertFormat(PixelFormat::PF_RGBA8_UINT);
		else if (format == PixelFormat::PF_RGB8_SINT)    image->convertFormat(PixelFormat::PF_RGBA8_SINT);
	}

	bool VKTexture2D::load()
	{
		MemoryReader memReader(getPath());
		if (memReader.getSize())
		{
			Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
			Image* image = Image::createFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
			if (image)
			{
				// vulkan doesn't support rgb format
				convertFormat(image);

				if (updateTexture2D(image->getPixelFormat(), TexUsage::TU_CPU_READ, image->getWidth(), image->getHeight(), image->getData(), 0))
				{
					EchoSafeDelete(image, Image);
					return true;
				}
				else
				{
					EchoSafeDelete(image, Image);
					return false;
				}
			}
		}

		return false;
	}

	bool VKTexture2D::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
	{
		m_isCompressed = false;
		m_compressType = Texture::CompressType_Unknown;
		m_width = width;
		m_height = height;
		m_depth = 1;
		m_pixFmt = format;
		m_numMipmaps = 1;

		VkImageUsageFlags vkUsageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VkFlags requirementsMask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;// VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_LINEAR;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
		if (createVkImage(getSamplerState(), m_pixFmt, m_width, m_height, m_depth, vkUsageFlags, requirementsMask, tiling, initialLayout))
		{
			ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
			Buffer buff(pixelsSize, data, false);
			setVkImageSurfaceData(0, m_pixFmt, m_usage, m_width, m_height, buff, false);

			return true;
		}
			
		EchoLogError("vulkan texture [%s] load failed", getPath().c_str());
		return false;
	}

    VKTextureRender::VKTextureRender(const String& name)
        : TextureRender(name)
    {

    }

    VKTextureRender::~VKTextureRender()
    {

    }

    bool VKTextureRender::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
    {
        m_width = width;
        m_height = height;
        m_usage = usage;
        m_isCompressed = false;
        m_compressType = Texture::CompressType_Unknown;
        m_depth = 1;
        m_pixFmt = format;
        m_numMipmaps = 1;

		VkImageUsageFlags vkUsageFlags = PixelUtil::IsDepth(format) ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		VkFlags requirementsMask = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
		VkImageLayout initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        createVkImage(getSamplerState(), m_pixFmt, m_width, m_height, m_depth, vkUsageFlags, requirementsMask, tiling, initialLayout);

		ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
		Buffer buff(pixelsSize, data, false);
        setVkImageSurfaceData(0, m_pixFmt, m_usage, m_width, m_height, buff, true);

        return true;
    }
}
