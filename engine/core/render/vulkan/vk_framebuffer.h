#pragma once

#include "base/frame_buffer.h"
#include "vk_render_base.h"
#include "vk_texture.h"

namespace Echo
{
    class VKFramebuffer
    {
    public:
        VKFramebuffer();
        virtual ~VKFramebuffer();

        // current frame buffer
        static VKFramebuffer* current();

    public:
        // get vk viewPort
        VkViewport getVkViewport() { return m_vkViewport; }

        // get vk render pass
        VkRenderPass getVkRenderPass() { return m_vkRenderPass; }

        // get vk view port create info
        VkPipelineViewportStateCreateInfo* getVkViewportStateCreateInfo() { return &m_vkViewportStateCreateInfo; }

        // get vk command buffer
        virtual VkCommandBuffer getVkCommandbuffer() { return nullptr; }

        // get vk frame buffer
        virtual VkFramebuffer getVkFramebuffer() { return VK_NULL_HANDLE; }

    protected:
        // clear color image
        virtual VkImage getVkColorImage() { return VK_NULL_HANDLE; }

    protected:
        // create render pass
        virtual void createVkRenderPass() {}
        virtual void destroyVkRenderPass();

    protected:
        VkRenderPass                        m_vkRenderPass = VK_NULL_HANDLE;
		VkRenderPassBeginInfo               m_vkRenderPassBeginInfo;
        VkViewport                          m_vkViewport;
        VkRect2D                            m_vkScissor;
        VkPipelineViewportStateCreateInfo   m_vkViewportStateCreateInfo;
        vector<VkFramebuffer>::type         m_vkFramebuffers;
    };

    class VKFramebufferOffscreen : public FrameBufferOffScreen, public VKFramebuffer
    {
    public:
        VKFramebufferOffscreen(ui32 width, ui32 height);
        virtual ~VKFramebufferOffscreen();

        // begin render
        virtual bool begin() override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;

    protected:
        // create vk frame buffer
        void createVkFramebuffers();
    };

    class VKFramebufferWindow : public FrameBufferWindow, public VKFramebuffer
    {
    public:
        VKFramebufferWindow();
        virtual ~VKFramebufferWindow();

        // begin render
        virtual bool begin() override;
        virtual bool end() override;

        // on resize
        virtual void onSize(ui32 width, ui32 height) override;

    public:
        // get vk command buffer
        virtual VkCommandBuffer getVkCommandbuffer() override { return m_vkCommandBuffers[m_imageIndex]; }

        // get vk frame buffer
        virtual VkFramebuffer getVkFramebuffer() override { return m_vkFramebuffers[m_imageIndex]; }

    private:
        // create render pass
        virtual void createVkRenderPass() override;

        // recreate swap chain
        void recreateVkSwapChain();
        void destroyVkSwapChain(VkSwapchainKHR vkSwapChain);

        // create vk frame buffer
        void createVkFramebuffers();
        void destroyVkFramebuffers();

        // vk command buffer
        void createVkCommandBuffers();
        void destroyVkCommandBuffers();

        // create semaphores
        void createVkSemaphores();

        // create fences
        void createVkFences();
        void destroyVkFences();

        // create swap chain
        void createSwapChain(VkDevice vkDevice);

        // create color image views
        void createVkColorImageViews(VkDevice vkDevice);
        void destroyVkColorImageViews();

        // create depth image view
        void createVkDepthImageView();
        void destroyVkDepthImageView();

        // check surface format
        VkSurfaceFormatKHR pickSurfaceSupportFormat();

        // create window surface
        void createVkSurface(void* handle);

        // clear color image
        virtual VkImage getVkColorImage() override { return m_vkSwapChainImages[m_imageIndex]; }

        // submit command buffer
        void submitCommandBuffer();

        // present
        void present();

    protected:
        ui32                            m_imageIndex = 0;
        vector<VkFence>::type           m_waitFences;
        VkSemaphore                     m_vkImageAvailableSemaphore;
        VkSemaphore                     m_vkRenderFinishedSemaphore;
        PixelFormat                     m_colorFormat = PF_BGRA8_UNORM;
        VkSurfaceKHR                    m_vkWindowSurface;
        VkSwapchainKHR				    m_vkSwapChain = VK_NULL_HANDLE;
        vector<VkImage>::type           m_vkSwapChainImages;
        vector<VkImageView>::type	    m_vkSwapChainImageViews;
        PixelFormat                     m_depthFormat = PF_D32_FLOAT;
        VKTextureRender*                m_vkDepthImageView = nullptr;
        vector<VkCommandBuffer>::type   m_vkCommandBuffers;    // https://vulkan.lunarg.com/doc/view/1.2.141.0/windows/chunked_spec/chap5.html
        VkQueue                         m_vkPresentQueue;
    };
}
