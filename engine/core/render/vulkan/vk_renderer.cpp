#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "vk_gpu_buffer.h"

namespace Echo
{
    VKRenderer::VKRenderer()
    {
    }

    VKRenderer::~VKRenderer()
    {
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

		createVkSwapChain();

		// window width height
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;

        // set view port
        Viewport viewport(0, 0, m_screenWidth, m_screenHeight);
        setViewport(&viewport);

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

	// create window surface
	void VKRenderer::createVkSurface(void* handle)
	{
		// create window surface
	#ifdef ECHO_PLATFORM_WINDOWS
		VkWin32SurfaceCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hwnd = (HWND)handle;
		createInfo.hinstance = GetModuleHandle(nullptr);

		// create surface
		if (VK_SUCCESS != vkCreateWin32SurfaceKHR(m_vkInstance, &createInfo, nullptr, &m_vkWindowSurface))
		{
			EchoLogError("Vulkan Renderer failed to create window surface!");
		}
	#endif
	}

	void VKRenderer::setViewport(Viewport* pViewport)
	{

	}

	void VKRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}

    Renderable* VKRenderer::createRenderable(const String& renderStage, ShaderProgram* material)
    {
        Renderable* renderable = EchoNew(VKRenderable(renderStage, material, m_renderableIdentifier++));
        ui32 id = renderable->getIdentifier();
        assert(!m_renderables.count(id));
        m_renderables[id] = renderable;

        return renderable;
    }

    ShaderProgram* VKRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }

    // create states
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

		vector<const char*>::type enabledExtensions;
		prepareVkExtensions(enabledExtensions);

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
		createInfo.enabledExtensionCount = enabledExtensions.size();
		createInfo.ppEnabledExtensionNames = enabledExtensions.data();
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
		float queuePrioritys[2] = { 1.f, 1.f};
		VkDeviceQueueCreateInfo queueCreateInfos[2];

		// graphics queue
		queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfos[0].pNext = nullptr;
		queueCreateInfos[0].queueFamilyIndex = getGraphicsQueueFamilyIndex();
		queueCreateInfos[0].queueCount = 1;
		queueCreateInfos[0].pQueuePriorities = &queuePrioritys[0];

		//// present queue
		//queueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		//queueCreateInfos[1].queueFamilyIndex = getPresentQueueFamilyIndex();
		//queueCreateInfos[1].queueCount = 1;
		//queueCreateInfos[1].pQueuePriorities = &queuePrioritys[1];

		// device features
		VkPhysicalDeviceFeatures deviceFeatures = {};

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pQueueCreateInfos = &queueCreateInfos[0];
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = 0;

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

	void VKRenderer::createVkSwapChain()
	{
		m_swapChain.create(m_vkDevice);
	}

	void VKRenderer::createVkValidation()
	{
		m_validation.create();
	}
}
