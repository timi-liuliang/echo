#pragma once

#include "base/texture.h"
#include "base/texture_render.h"
#include "vk_render_base.h"

namespace Echo
{
    class VKTexture2D : public Texture
    {
    public:
        virtual ~VKTexture2D();
    };

    class VKTextureRender : public TextureRender
    {
    public:
        VKTextureRender(const String& name);
        virtual ~VKTextureRender();

		// update texture by rect
        virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) override;

    protected:
        // VkImage
        void createVkImage();
        void destroyVkImage();

        // VkImageMemory
        void createVkImageMemory();
        void destroyVkImageMemory();

        // VkImageView
        void createVkImageView();
        void destroyVkImageView();

		// set surface data
		void setVkImageSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff);

    protected:
        VkImage             m_vkImage = VK_NULL_HANDLE;
        VkDeviceMemory      m_vkImageMemory = VK_NULL_HANDLE;
        VkImageView         m_vkImageView = VK_NULL_HANDLE;
    };
}
