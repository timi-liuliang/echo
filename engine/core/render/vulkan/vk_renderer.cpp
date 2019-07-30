#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"

namespace Echo
{
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		EchoLogError(pCallbackData->pMessage);

		return VK_FALSE;
	}

    VKRenderer::VKRenderer()
    {
		createVkInstance();
    }
    
    VKRenderer::~VKRenderer()
    {
		vkDestroyInstance(m_vkInstance, nullptr);
    }

    bool VKRenderer::initialize(const Config& config)
    {
        m_screenWidth = config.screenWidth;
        m_screenHeight = config.screenHeight;
            
        // set view port
        Viewport viewport(0, 0, m_screenWidth, m_screenHeight);
        setViewport(&viewport);
        
        return true;
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

	void VKRenderer::enumerateVkValidationLayers()
	{
#if FALSE
		ui32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		m_vkLayers.resize(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, m_vkLayers.data());
#endif
	}

	void VKRenderer::prepareVkValidationLayers(vector<const char*>::type& validationLayers)
	{
		validationLayers = { "VK_LAYER_KHRONOS_validation" };
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
	}

	void VKRenderer::createVkInstance()
	{
		enumerateVkValidationLayers();
		enumerateVkExtensions();

		vector<const char*>::type enabledExtensions;
		prepareVkExtensions(enabledExtensions);

		vector<const char*>::type validationLayers;
		prepareVkValidationLayers(validationLayers);

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
}
