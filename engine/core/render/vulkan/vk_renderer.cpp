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

		createVkCommandPool();
        createVkCommandBuffer();
        executeBeginVkCommandBuffer();

        createVkDepthBuffer(640, 480);

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

	void VKRenderer::setViewport(Viewport* pViewport)
	{

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

	void VKRenderer::createVkSwapChain()
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
        commandBufferBeginInfo.flags = 0;
        commandBufferBeginInfo.pInheritanceInfo = nullptr;

        if (VK_SUCCESS != vkBeginCommandBuffer(m_vkCommandBuffer, &commandBufferBeginInfo))
        {
            EchoLogError("vulkan begin command buffer failed.");
        }
    }

    void VKRenderer::createVkDepthBuffer(ui32 width, ui32 height)
    {
#ifdef ECHO_PLATFORM_ANDROID
        m_vkDepth.format = VK_FORMAT_D24_UNORM_S8_UINT;
#else
        m_vkDepth.format = VK_FORMAT_D16_UNORM; // VK_FORMAT_D32_SFLOAT
#endif

        //const VkFormat depth_format = info.depth.format;
        //VkFormatProperties props;
        //vkGetPhysicalDeviceFormatProperties(info.gpus[0], depth_format, &props);
        //if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        //    image_info.tiling = VK_IMAGE_TILING_LINEAR;
        //}
        //else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        //    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        //}
        //else {
        //    /* Try other depth formats? */
        //    std::cout << "depth_format " << depth_format << " Unsupported.\n";
        //    exit(-1);
        //}

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.pNext = nullptr;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = m_vkDepth.format;
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

    bool VKRenderer::present()
    {
        VkPresentInfoKHR present;
        present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present.pNext = nullptr;
        present.swapchainCount = 1;
        present.pSwapchains = m_swapChain.getVkSwapchain();
        present.pImageIndices = 0;
        present.pWaitSemaphores = nullptr;
        present.waitSemaphoreCount = 0;
        present.pResults = nullptr;

        //if (VK_SUCCESS != vkQueuePresentKHR(m_vkPresentQueue, &present))
        {
            //EchoLogError("vulkan present failed");

            return false;
        }

        return true;
    }
}
