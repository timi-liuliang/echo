#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "vk_gpu_buffer.h"
#include "vk_render_view.h"
#include "vk_framebuffer_window.h"
#include "vk_texture.h"

namespace Echo
{
    VKRenderer::VKRenderer()
    {
    }

    VKRenderer::~VKRenderer()
    {
        vkDestroySemaphore(m_vkDevice, m_vkRenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphore, nullptr);

		m_validation.cleanup();
		vkDestroyDevice(m_vkDevice, nullptr);
		vkDestroySurfaceKHR(m_vkInstance, m_vkWindowSurface, nullptr);
		vkDestroyInstance(m_vkInstance, nullptr);
    }

    bool VKRenderer::initialize(const Config& config)
    {
		createVkInstance();

		createVkValidation();

		createVkSurface((void*)config.windowHandle);

		pickVkPhysicalDevice();

		createVkLogicalDevice();

		createSwapChain();

		createVkCommandPool();
        createVkCommandBuffer();
        executeBeginVkCommandBuffer();

        createVkSemaphores();

        createVkDepthBuffer(640, 480);

        createVkRenderPass();

		// window width height
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;

        // set view port
        Viewport viewport(0, 0, m_screenWidth, m_screenHeight);

        return true;
    }

	GPUBuffer* VKRenderer::createVertexBuffer(Dword usage, const Buffer& buff)
	{
		return EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_VERTEX, usage, buff));
	}

	GPUBuffer* VKRenderer::createIndexBuffer(Dword usage, const Buffer& buff)
	{
		return EchoNew(VKBuffer(GPUBuffer::GPUBufferType::GBT_INDEX, usage, buff));
	}

    Texture* VKRenderer::createTexture2D(const String& name)
    {
        return EchoNew(VKTexture2D);
    }

	void VKRenderer::createVkSurface(void* handle)
	{
		// create window surface
	#ifdef ECHO_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.hwnd = (HWND)handle;
		createInfo.hinstance = GetModuleHandle(nullptr);

		// create surface
		if (VK_SUCCESS != vkCreateWin32SurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_vkWindowSurface))
		{
			EchoLogError("Vulkan Renderer failed to create window surface!");
		}
	#elif defined(ECHO_PLATFORM_ANDROID)
		VkAndroidSurfaceCreateInfoKHR createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.window = AndroidGetApplicationWindow();
		if (VK_SUCCESS != vkCreateAndroidSurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_vkWindowSurface))
		{
			EchoLogError("Vulkan Renderer failed to create window surface!");
		}
	#endif
	}

	void VKRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}

    Renderable* VKRenderer::createRenderable(const String& renderStage, ShaderProgram* material)
    {
        static ui32 id = 1;
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, id++));
        m_renderables[id] = renderable;

        return renderable;
    }

    ShaderProgram* VKRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }

    RasterizerState* VKRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(VKRasterizerState);
    }

    DepthStencilState* VKRenderer::createDepthStencilState(const DepthStencilState::DepthStencilDesc& desc)
    {
        return nullptr;
        //return EchoNew(VKDepthStencilState);
    }

    BlendState* VKRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(VKBlendState);
    }

    const SamplerState* VKRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(VKSamplerState);
    }

	void VKRenderer::enumerateVkExtensions()
	{
#if FALSE
		ui32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		m_vkExtensions.resize(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, m_vkExtensions.data());
#endif
	}

	void VKRenderer::prepareVkExtensions(vector<const char*>::type& extensions)
	{
		extensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	#if defined(_WIN32)
		extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#elif defined(__ANDROID__)
		extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#elif defined(__linux__)
		extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
	#endif

	#if defined(ECHO_EDITOR_MODE)
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	#endif
	}

	void VKRenderer::createVkInstance()
	{
		enumerateVkExtensions();
		prepareVkExtensions(m_enabledExtensions);

		vector<const char*>::type validationLayers;
		m_validation.prepareVkValidationLayers(validationLayers);

		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "echo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "echo";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;
		appInfo.pNext = nullptr;

		VkInstanceCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = m_enabledExtensions.size();
		createInfo.ppEnabledExtensionNames = m_enabledExtensions.data();
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.enabledLayerCount = 0;

		// create instance
		if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, &m_vkInstance))
		{
			EchoLogError("create vulkan instance failed");
		}
	}

	i32 VKRenderer::calcVkPhysicalDeviceScore(const VkPhysicalDevice& device)
	{
		// query device properties
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		// query device features
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		int score = 0;

		// Discrete GPUs have a significant performance advantage
		score += deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? 1000 : 0;

		// Maximum possible size of textures affects graphics quality
		score += deviceProperties.limits.maxImageDimension2D;

		// Application can't function without geometry shaders
		score += deviceFeatures.geometryShader ? 1000 : 0;

		return score;
	}

	void VKRenderer::pickVkPhysicalDevice()
	{
		ui32 deviceCount = 0;
		vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);
		if(deviceCount>0)
		{
			vector<VkPhysicalDevice>::type devices(deviceCount);
			vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

			i32 score = 0;
			for(VkPhysicalDevice& device : devices)
			{
				i32 curScore = calcVkPhysicalDeviceScore(device);
				if(curScore>score)
				{
					m_vkPhysicalDevice = device;
					score = curScore;
				}
			}
		}

		// output error
		if(m_vkPhysicalDevice!=VK_NULL_HANDLE)
		{
			enumerateQueueFamalies();
		}
		else
		{
			EchoLogError("Failed to find GPUs with Vulkan support!");
		}
	}

	void VKRenderer::enumerateQueueFamalies()
	{
		ui32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, nullptr);

		m_vkQueueFamilies.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(m_vkPhysicalDevice, &queueFamilyCount, m_vkQueueFamilies.data());
	}

	const ui32 VKRenderer::getGraphicsQueueFamilyIndex()
	{
		for(size_t i=0; i<m_vkQueueFamilies.size(); i++)
		{
			if(m_vkQueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				return i;
		}

		return -1;
	}

	const ui32 VKRenderer::getPresentQueueFamilyIndex()
	{
		for(size_t i=0; i<m_vkQueueFamilies.size(); i++)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, m_vkWindowSurface, &presentSupport);
			if(presentSupport)
				return i;
		}

		return -1;
	}

	void VKRenderer::createVkLogicalDevice()
	{
		// device extensions
		vector<const char*>::type deviceExtensionNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		// priorities
		float queuePrioritys[2] = { 1.f, 1.f};

		// graphics queue
		VkDeviceQueueCreateInfo queueCreateInfos;
		queueCreateInfos.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos.pNext = nullptr;
		queueCreateInfos.queueFamilyIndex = getGraphicsQueueFamilyIndex();
		queueCreateInfos.queueCount = 1;
		queueCreateInfos.pQueuePriorities = &queuePrioritys[0];

		// device features
		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pQueueCreateInfos = &queueCreateInfos;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = deviceExtensionNames.size();
		createInfo.ppEnabledExtensionNames = deviceExtensionNames.data();

		// create logical device and retirve graphics queue
		if (VK_SUCCESS == vkCreateDevice(m_vkPhysicalDevice, &createInfo, nullptr, &m_vkDevice))
		{
			vkGetDeviceQueue(m_vkDevice, getGraphicsQueueFamilyIndex(), 0, &m_vkGraphicsQueue);
			vkGetDeviceQueue(m_vkDevice, getPresentQueueFamilyIndex(), 0, &m_vkPresentQueue);
		}
		else
    	{
			EchoLogError("Failed to create vulkan logical device!");
		}
	}

	void VKRenderer::createSwapChain()
	{
		m_swapChain.create(m_vkDevice);
	}

	void VKRenderer::createVkValidation()
	{
		m_validation.create();
	}

	void VKRenderer::createVkCommandPool()
	{
		VkCommandPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createInfo.pNext = NULL;
		createInfo.queueFamilyIndex = getGraphicsQueueFamilyIndex();
		createInfo.flags = 0;

		if (VK_SUCCESS != vkCreateCommandPool(m_vkDevice, &createInfo, nullptr, &m_vkCommandPool))
		{
			EchoLogError("Vulkan create command pool failed");
		}
	}

    void VKRenderer::createVkCommandBuffer()
    {
        VkCommandBufferAllocateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        createInfo.pNext = nullptr;
        createInfo.commandPool = m_vkCommandPool;
        createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        createInfo.commandBufferCount = 1;

        if (VK_SUCCESS != vkAllocateCommandBuffers(m_vkDevice, &createInfo, &m_vkCommandBuffer))
        {
            EchoLogError("vulkan create command buffer failed");
        }
    }

    void VKRenderer::executeBeginVkCommandBuffer()
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo = {};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = nullptr;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (VK_SUCCESS != vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo))
        {
            EchoLogError("vulkan begin command buffer failed.");
        }

        // clear
        VkClearColorValue clearColor = { Renderer::BGCOLOR.r, Renderer::BGCOLOR.g, Renderer::BGCOLOR.b, Renderer::BGCOLOR.a };
        VkClearValue clearValue = {};
        clearValue.color = clearColor;

        VkImageSubresourceRange imageRange = {};
        imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageRange.levelCount = 1;
        imageRange.layerCount = 1;

        vkCmdClearColorImage(m_vkCommandBuffer, m_swapChain.getVkImage(0), VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);

        vkEndCommandBuffer(m_vkCommandBuffer);
    }

    void VKRenderer::createVkSemaphores()
    {
        VkSemaphoreCreateInfo semaphoreCreateInfo = {};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, nullptr, &m_vkImageAvailableSemaphore) != VK_SUCCESS ||
            vkCreateSemaphore(m_vkDevice, &semaphoreCreateInfo, nullptr, &m_vkRenderFinishedSemaphore) != VK_SUCCESS)
        {
            EchoLogError("vulkan failed to create semaphores!");
        }
    }

    void VKRenderer::createVkDepthBuffer(ui32 width, ui32 height)
    {
        RenderView* depthView = EchoNew(VKRenderView(width, height, PF_D24_UNORM_S8_UINT));
    }

    bool VKRenderer::present()
    {
        ui32 imageIndex = 0;

        VkPresentInfoKHR present;
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.pNext = nullptr;
        present.swapchainCount = 1;
        present.pSwapchains = m_swapChain.getVkSwapchain();
        present.pImageIndices = &imageIndex;
        present.pWaitSemaphores = nullptr;
        present.waitSemaphoreCount = 0;
        present.pResults = nullptr;

        if (VK_SUCCESS != vkQueuePresentKHR(m_vkPresentQueue, &present))
        {
            EchoLogError("vulkan present failed");

            return false;
        }

        return true;
    }

    FrameBuffer* VKRenderer::getWindowFrameBuffer()
    {
        if (!m_windowFramebuffer)
        {
            m_windowFramebuffer = EchoNew(VKFramebufferWindow(m_screenWidth, m_screenHeight));
        }

        return m_windowFramebuffer;
    }

    void VKRenderer::createVkRenderPass()
    {
        VkAttachmentReference attachRef = {};
        attachRef.attachment = 0;
        attachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDesc = {};
        subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDesc.colorAttachmentCount = 1;
        subpassDesc.pColorAttachments = &attachRef;

        VkAttachmentDescription attachDesc = {};
        attachDesc.format = m_core.GetSurfaceFormat().format;
        attachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachDesc.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &attachDesc;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDesc;

        if(VK_SUCCESS != vkCreateRenderPass(m_vkDevice, &renderPassCreateInfo, nullptr, &m_vkRenderPass))
        {
            EchoLogError("vulkan create render pass failed.");
        }
    }
}
