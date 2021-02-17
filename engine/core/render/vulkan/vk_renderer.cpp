#include "base/mesh/mesh.h"
#include "base/pipeline/render_pipeline.h"
#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "vk_gpu_buffer.h"
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
		vkDestroyDescriptorPool(VKRenderer::instance()->getVkDevice(), m_vkDescriptorPool, nullptr);

		m_validation.cleanup();
		vkDestroyDevice(m_vkDevice, nullptr);
		vkDestroyInstance(m_vkInstance, nullptr);
    }

    VKRenderer* VKRenderer::instance()
    {
        return g_inst;
    }

    bool VKRenderer::initialize(const Settings& settings)
    {
		m_settings = settings;

		createVkInstance();

		createVkValidation();

		pickVkPhysicalDevice();

		createVkLogicalDevice();

		createVkCommandPool();

		createVkDescriptorPool();

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

	TextureRender* VKRenderer::createTextureRender(const String& name)
	{ 
		return EchoNew(VKTextureRender); 
	}

	void VKRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{

	}

    Renderable* VKRenderer::createRenderable()
    {
        static ui32 id = 0; id++;
        Renderable* renderable = EchoNew(VKRenderable(id));
        m_renderables[id] = renderable;

        return renderable;
    }

    ShaderProgram* VKRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }

    RasterizerState* VKRenderer::createRasterizerState(const RasterizerState::RasterizerDesc& desc)
    {
        return EchoNew(VKRasterizerState(desc));
    }

    DepthStencilState* VKRenderer::createDepthStencilState()
    {
        return EchoNew(VKDepthStencilState);
    }

    BlendState* VKRenderer::createBlendState(const BlendState::BlendDesc& desc)
    {
        return EchoNew(VKBlendState(desc));
    }

    MultisampleState* VKRenderer::createMultisampleState()
    {
        return EchoNew(VKMultisampleState);
    }

    const SamplerState* VKRenderer::getSamplerState(const SamplerState::SamplerDesc& desc)
    {
        return EchoNew(VKSamplerState(desc));
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
		extensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#elif defined(__ANDROID__)
		extensions.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
	#elif defined(__linux__)
		extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
	#endif

	#if defined(ECHO_EDITOR_MODE)
		extensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
        VKDebug(vkCreateInstance(&createInfo, nullptr, &m_vkInstance));
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
            VKDebug(vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhysicalDevice, i, vkSurface, &presentSupport));
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
        VKDebug(vkCreateDevice(m_vkPhysicalDevice, &createInfo, nullptr, &m_vkDevice));
		vkGetDeviceQueue(m_vkDevice, getGraphicsQueueFamilyIndex(), 0, &m_vkGraphicsQueue);
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

        VKDebug(vkCreateCommandPool(m_vkDevice, &createInfo, nullptr, &m_vkCommandPool));
	}

	void VKRenderer::createVkDescriptorPool()
	{
		array<VkDescriptorPoolSize, 1> typeCounts;
		typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		typeCounts[0].descriptorCount = 512;

		// For additional type you need to add new entries in the type count list
		//typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//typeCounts[1].descriptorCount = 2;

		// Create the global descriptor pool
		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = typeCounts.size();
		descriptorPoolInfo.pPoolSizes = typeCounts.data();
		descriptorPoolInfo.maxSets = 512;

		VKDebug(vkCreateDescriptorPool( m_vkDevice, &descriptorPoolInfo, nullptr, &m_vkDescriptorPool));
	}

    void VKRenderer::onSize(int width, int height)
    {
		m_screenWidth = width;
		m_screenHeight = height;

        // render target
        RenderPipeline::current()->onSize(width, height);
    }

    void VKRenderer::draw(Renderable* renderable)
    {
		VKFramebuffer* currentFrameBuffer = VKFramebuffer::current();
		if (currentFrameBuffer)
		{
			VKRenderable* vkRenderable = ECHO_DOWN_CAST<VKRenderable*>(renderable);
			if (vkRenderable->createVkPipeline(currentFrameBuffer))
			{
				ShaderProgram* shaderProgram = renderable->getMaterial()->getShader();
				VkCommandBuffer vkCommandbuffer = currentFrameBuffer->getVkCommandbuffer();

				vkCmdBindPipeline(vkCommandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkRenderable->getVkPipeline());
				vkRenderable->bindShaderParams(vkCommandbuffer);
				vkRenderable->bindGeometry(vkCommandbuffer);

				MeshPtr mesh = renderable->getMesh();
				if (mesh->getIndexBuffer())
				{
					ui32 idxCount = mesh->getIndexCount();
					ui32 idxOffset = mesh->getStartIndex();

					vkCmdDrawIndexed(vkCommandbuffer, idxCount, 1, idxOffset, 0, 0);
				}
				else
				{
					ui32 vertCount = mesh->getVertexCount();
					ui32 startVert = mesh->getStartVertex();

					vkCmdDraw(vkCommandbuffer, vertCount, 1, startVert, 0);
				}
			}
		}
    }

    bool VKRenderer::present()
    {
        return false;
    }
}
