#include "base/mesh/mesh.h"
#include "base/pipeline/render_pipeline.h"
#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "vk_gpu_buffer.h"
#include "vk_framebuffer.h"
#include "vk_texture.h"

extern "C"
{
#include <thirdparty/vulkan/vulkan-loader/loader/loader.h>
}

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

		// initialize vulkan loader
		loader_initialize();

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
        return EchoNew(VKTexture2D(name));
    }

	TextureRender* VKRenderer::createTextureRender(const String& name)
	{ 
		return EchoNew(VKTextureRender(name)); 
	}

	void VKRenderer::setTexture(ui32 index, Texture* texture, bool needUpdate)
	{
		m_currentTextures[index] = texture;
	}

	VKTexture* VKRenderer::getTexture(ui32 index)
	{
		return ECHO_DOWN_CAST<VKTexture*>(m_currentTextures[index]);
	}

    Renderable* VKRenderer::createRenderable()
    {
        static ui32 id = 0; id++;
        Renderable* renderable = EchoNew(VKRenderable(id));
        m_renderables[id] = renderable;

        return renderable;
    }

	void VKRenderer::getDepthRange(Vector2& vec)
	{
		vec.x = 0.0f;
		vec.y = 1.0f;
	}

	// http://anki3d.org/vulkan-coordinate-system/
	void VKRenderer::convertMatView(Matrix4& mat)
	{
		mat.m01 = -mat.m01;
		mat.m11 = -mat.m11;
		mat.m21 = -mat.m21;
		mat.m31 = -mat.m31;
	}

	void VKRenderer::unproject(Vector3& worldPos, const Vector3& screenPos, const Matrix4& matVP, Viewport* viewport)
	{
		Viewport viewPort(0, 0, getWindowWidth(), getWindowHeight());
		if (!viewport)
			viewport = &viewPort;

		Matrix4 matVPInv = matVP;
		matVPInv.detInverse();

		Vector4 vWSPos = Vector4(screenPos, 1.0f);
		vWSPos.x = (screenPos.x - viewport->getLeft()) / (Real)viewport->getWidth() * 2.0f - 1.0f;
		vWSPos.y = -(1.f - (screenPos.y - viewport->getTop()) / viewport->getHeight() * 2.0f);

		Vector4 vWorld = vWSPos * matVPInv;
		vWorld /= vWorld.w;

		worldPos = (Vector3)vWorld;
	}

    ShaderProgram* VKRenderer::createShaderProgram()
    {
        return EchoNew(VKShaderProgram);
    }

    RasterizerState* VKRenderer::createRasterizerState()
    {
        return EchoNew(VKRasterizerState);
    }

    DepthStencilState* VKRenderer::createDepthStencilState()
    {
        return EchoNew(VKDepthStencilState);
    }

    BlendState* VKRenderer::createBlendState()
    {
        return EchoNew(VKBlendState);
    }

    MultisampleState* VKRenderer::createMultisampleState()
    {
        return EchoNew(VKMultisampleState);
    }

    SamplerState* VKRenderer::createSamplerState()
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
		createInfo.flags = 0;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = m_enabledExtensions.size();
		createInfo.ppEnabledExtensionNames = m_enabledExtensions.data();
		createInfo.enabledLayerCount = validationLayers.size();
		createInfo.ppEnabledLayerNames = validationLayers.data();

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
		queueCreateInfos.flags = 0;
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
		createInfo.pNext = nullptr;
		createInfo.queueFamilyIndex = getGraphicsQueueFamilyIndex();
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VKDebug(vkCreateCommandPool(m_vkDevice, &createInfo, nullptr, &m_vkCommandPool));
	}

	void VKRenderer::createVkDescriptorPool()
	{
		array<VkDescriptorPoolSize, 2> typeCounts;
		typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		typeCounts[0].descriptorCount = 512;

		// For additional type you need to add new entries in the type count list
		typeCounts[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		typeCounts[1].descriptorCount = 512;

		// Create the global descriptor pool
		VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
		descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolInfo.pNext = nullptr;
		descriptorPoolInfo.poolSizeCount = typeCounts.size();
		descriptorPoolInfo.pPoolSizes = typeCounts.data();
		descriptorPoolInfo.maxSets = 512;

		VKDebug(vkCreateDescriptorPool( m_vkDevice, &descriptorPoolInfo, nullptr, &m_vkDescriptorPool));
	}

	ui32 VKRenderer::findVkMemoryType(ui32 typeBits, VkFlags requirementsMask)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &memProperties);

		for (ui32 i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((memProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
					return i;
			}

			typeBits >>= 1;
		}

		// no memory types matched, return failure
		return 0;
	}

	VkCommandBuffer VKRenderer::createVkCommandBuffer()
	{
		VkCommandBufferAllocateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.commandPool = getVkCommandPool();
		createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		createInfo.commandBufferCount = 1;

		VkCommandBuffer vkCommandBuffer;
		VKDebug(vkAllocateCommandBuffers(VKRenderer::instance()->getVkDevice(), &createInfo, &vkCommandBuffer));
		
		return vkCommandBuffer;
	}

	void VKRenderer::flushVkCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
	{
		if (commandBuffer)
		{
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffer;

			// Create fence to ensure that the command buffer has finished executing
			VkFenceCreateInfo fenceCreateInfo = {};
			fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			fenceCreateInfo.pNext = nullptr;
			fenceCreateInfo.flags = 0;

			VkFence fence;
			VKDebug(vkCreateFence(getVkDevice(), &fenceCreateInfo, nullptr, &fence));

			// Submit to the queue
			VKDebug(vkQueueSubmit(queue, 1, &submitInfo, fence));

			// Wait for the fence to signal that command buffer has finished executing
			VKDebug(vkWaitForFences(getVkDevice(), 1, &fence, VK_TRUE, UINT64_MAX));
			vkDestroyFence(getVkDevice(), fence, nullptr);

			if (free)
				vkFreeCommandBuffers(getVkDevice(), getVkCommandPool(), 1, &commandBuffer);
		}
	}

    void VKRenderer::onSize(int width, int height)
    {
		m_screenWidth = width;
		m_screenHeight = height;

        // render target
        RenderPipeline::current()->onSize(width, height);
    }

    void VKRenderer::draw(Renderable* renderable, FrameBufferPtr& frameBuffer)
    {
		VKFramebuffer* currentFrameBuffer = ECHO_DOWN_CAST<VKFramebuffer*>(frameBuffer.ptr());
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
