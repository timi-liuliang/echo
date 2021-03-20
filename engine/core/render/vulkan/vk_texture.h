#pragma once

#include "base/texture.h"
#include "base/texture_render.h"
#include "vk_render_base.h"
#include "vk_render_state.h"

namespace Echo
{
    class VKTexture
    {
    public:
        VKTexture();
        virtual ~VKTexture();

		// get vk image view
		VkImageView getVkImageView() { return m_vkImageView; }

        // get vk descriptor image info
        VkDescriptorImageInfo* getVkDescriptorImageInfo() { return m_vkDescriptorImageInfo.sampler ? &m_vkDescriptorImageInfo : nullptr; }

	protected:
		// VkImage
		bool createVkImage(SamplerStatePtr samplerState, PixelFormat format, i32 width, i32 height, i32 depth, VkImageUsageFlags usage, VkFlags requirementsMask, VkImageTiling tiling, VkImageLayout initialLayout);
		void destroyVkImage();

		// VkImageMemory
		void createVkImageMemory(VkFlags requirementsMask);
		void destroyVkImageMemory();

		// VkImageView
		void createVkImageView(PixelFormat format);
		void destroyVkImageView();

        // Descriptor info
        void createDescriptorImageInfo(SamplerStatePtr samplerState);

		// set surface data
		void setVkImageSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff, bool isUseStaging);

        // tool set image layout
        void setImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

	protected:
		VkImage                 m_vkImage = VK_NULL_HANDLE;
		VkDeviceMemory          m_vkImageMemory = VK_NULL_HANDLE;
		VkImageView             m_vkImageView = VK_NULL_HANDLE;
        VkDescriptorImageInfo   m_vkDescriptorImageInfo = {};
    };

    class VKTexture2D : public Texture, public VKTexture
    {
    public:
        VKTexture2D(const String& pathName);
        virtual ~VKTexture2D();

        // type
        virtual TexType getType() const override { return TT_2D; }

        // get vulkan texture
        
        // get vulkan sampler
        
        // load
        virtual bool load() override;

		// update data
        virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size);

	private:
		// convert format
		void convertFormat(Image* image);
    };

    class VKTextureRender : public TextureRender, public VKTexture
    {
    public:
        VKTextureRender(const String& name);
        virtual ~VKTextureRender();

		// update texture by rect
        virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) override;
    };
}
