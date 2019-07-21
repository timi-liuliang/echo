#include "vk_renderer.h"
#include "vk_renderable.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"

namespace Echo
{
    VKRenderer::VKRenderer()
    {
		createVkInstance();
    }
    
    VKRenderer::~VKRenderer()
    {
        
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

	void VKRenderer::createVkInstance()
	{
		VkApplicationInfo appInfo;
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "echo";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "echo";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_1;

		VkInstanceCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = 0;
		createInfo.enabledLayerCount = 0;

		// create instance
		if (VK_SUCCESS != vkCreateInstance(&createInfo, nullptr, m_vkInstance))
		{
			EchoLogError("create vulkan instance failed");
		}
	}
}
