#pragma once

#include "engine/core/render/interface/Renderer.h"
#include "vk_render_base.h"
#include "vk_validation.h"
#include "vk_framebuffer.h"

namespace Echo
{
    class GPUBuffer;
	class VKRenderer: public Renderer
	{
		typedef vector<const char*>::type				Extensions;
		typedef vector<VkExtensionProperties>::type     ExtensionProperties;
        typedef vector<VkQueueFamilyProperties>::type   QueueFamilies;

	public:
		VKRenderer();
		virtual ~VKRenderer();

        // get
        static VKRenderer* instance();

		// initialize
        virtual bool initialize(const Config& config) override;

        // get type
        virtual Type getType() { return Renderer::Type::Vulkan; }

        // create buffer
        virtual GPUBuffer*	createVertexBuffer(Dword usage, const Buffer& buff) override;
        virtual GPUBuffer*	createIndexBuffer(Dword usage, const Buffer& buff) override;

        // create texture
        virtual Texture*     createTexture2D(const String& name) override;
        virtual TextureCube* createTextureCube(const String& name) override {return nullptr; }

        // create views
        virtual RenderView*  createRenderView(ui32 width, ui32 height, PixelFormat pixelFormat) override { return nullptr; }
        virtual FrameBuffer* createFramebuffer(ui32 id, ui32 width, ui32 height) override { return EchoNew(VKFramebufferOffscreen(id, width, height));}

        // create states
        virtual RasterizerState* createRasterizerState(const RasterizerState::RasterizerDesc& desc) override;
        virtual DepthStencilState* createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)override;
        virtual BlendState* createBlendState(const BlendState::BlendDesc& desc) override;
        virtual MultisampleState* createMultisampleState() override;
        virtual const SamplerState* getSamplerState(const SamplerState::SamplerDesc& desc) override;

        // create shaders
        virtual ShaderProgram* createShaderProgram() override;

		// create renderable
        virtual Renderable* createRenderable(const String& renderStage, ShaderProgram* material) override;

        // convert matrix
        virtual void convertMatOrho(Matrix4& mat, const Matrix4& matOrth, Real zn, Real zf) override {}
        virtual void convertMatProj(Matrix4& mat, const Matrix4& matProj) override {}

		// scissor operate
		virtual void scissor(ui32 left, ui32 top, ui32 width, ui32 height) override {}
		virtual void endScissor() override {}

		// on size
		virtual void onSize(int width, int height) override {}

		// draw
        virtual void draw(Renderable* renderable) override;

		// present
        virtual bool present() override;

    public:
        // set texture
        virtual void setTexture(ui32 index, Texture* texture, bool needUpdate = false) override;

    public:
        // get max stage number
        virtual ui32 getMaxStageNum() const override { return 32;}

        // get depth range
        virtual void getDepthRange(Vector2& vec) override {}

        // get screen width and height
        virtual ui32 getWindowWidth() override { return m_framebufferWindow->getWidth();}
        virtual ui32 getWindowHeight() override { return m_framebufferWindow->getHeight();}

        // get screen frame buffer
        virtual FrameBuffer* getWindowFrameBuffer() override;

		// get view port
		virtual void getViewportReal(Viewport& pViewport) override {}

	public:
		VkInstance getVkInstance() { return m_vkInstance; }

		// get physical device
		VkPhysicalDevice getVkPhysicalDevice() { return m_vkPhysicalDevice; }

		// get logical device
		VkDevice getVkDevice() { return m_vkDevice; }

		// get queue family index
		const ui32 getGraphicsQueueFamilyIndex();
		const ui32 getPresentQueueFamilyIndex(VkSurfaceKHR vkSurface);

        // get queue
        VkQueue getVkGraphicsQueue() { return m_vkGraphicsQueue; }

        // get clear command buffer
        VkCommandPool getVkCommandPool() { return m_vkCommandPool; }

        // semaphore
        VkSemaphore getImageAvailableSemaphore() { return m_vkImageAvailableSemaphore; }
        VkSemaphore getRenderFinishedSemaphore() { return m_vkRenderFinishedSemaphore; }

	private:
		// create vk instance
		void createVkInstance();

		// vk extensions
		void enumerateVkExtensions();
		void prepareVkExtensions(vector<const char*>::type& extensions);

        // choose a physical device
        void pickVkPhysicalDevice();
        i32  calcVkPhysicalDeviceScore(const VkPhysicalDevice& device);

        // queue famalies
        void enumerateQueueFamalies();

        // create vk logical device
        void createVkLogicalDevice();

		// set up validation
		void createVkValidation();

		// create command pool
		void createVkCommandPool();

        // create semaphores
        void createVkSemaphores();

    private:
        FrameBuffer*        m_framebufferWindow = nullptr;
		Extensions			m_enabledExtensions;
		VkInstance			m_vkInstance;
		ExtensionProperties	m_vkExtensions;
        VkPhysicalDevice    m_vkPhysicalDevice = nullptr;
        QueueFamilies       m_vkQueueFamilies;
        VkDevice            m_vkDevice = nullptr;
		VKValidation		m_validation;
        VkQueue             m_vkGraphicsQueue;
		VkCommandPool		m_vkCommandPool;
        VkSemaphore         m_vkImageAvailableSemaphore;
        VkSemaphore         m_vkRenderFinishedSemaphore;
	};
}
