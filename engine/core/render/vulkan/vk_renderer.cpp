#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "vk_gpu_buffer.h"
#include "vk_render_view.h"
#include "vk_framebuffer.h"
#include "vk_texture.h"

namespace Echo
{
    VKRenderer* g_inst = nullptr;

    VKRenderer::VKRenderer()
    {
        g_inst = this;
    }

    VKRenderer::~VKRenderer()
    {
        vkDestroySemaphore(m_vkDevice, m_vkRenderFinishedSemaphore, nullptr);
        vkDestroySemaphore(m_vkDevice, m_vkImageAvailableSemaphore, nullptr);

		m_validation.cleanup();
		vkDestroyDevice(m_vkDevice, nullptr);
		vkDestroyInstance(m_vkInstance, nullptr);
    }

    VKRenderer* VKRenderer::instance()
    {
        return g_inst;
    }

    bool VKRenderer::initialize(const Config& config)
    {
		createVkInstance();

		createVkValidation();

		pickVkPhysicalDevice();

		createVkLogicalDevice();

		createVkCommandPool();

        createVkSemaphores();

        // window frame buffer
        m_framebufferWindow = EchoNew(VKFramebufferWindow(config.m_windowWidth, config.m_windowHeight, (void*)config.m_windowHandle));

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

    const ui32 VKRenderer::getPresentQueueFamilyIndex(VkSurfaceKHR vkSurface)
    {
        for (size_t i = 0; i < m_vkQueueFamilies.size(); i++)
        {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, vkSurface, &presentSupport);
            if (presentSupport)
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
		}
		else
    	{
			EchoLogError("Failed to create vulkan logical device!");
		}
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

    bool VKRenderer::present()
    {
        return false;
    }

    FrameBuffer* VKRenderer::getWindowFrameBuffer()
    {
        return m_framebufferWindow;
    }
}
