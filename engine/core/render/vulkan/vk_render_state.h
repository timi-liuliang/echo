#pragma once

#include "engine/core/render/interface/RenderState.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKBlendState : public BlendState
	{
	public:
        VKBlendState(const BlendDesc &desc);
        virtual ~VKBlendState(){}

        // get vk create info
        const VkPipelineColorBlendStateCreateInfo* getVkCreateInfo() { return &m_vkCreateInfo; }

    private:
        VkPipelineColorBlendStateCreateInfo     m_vkCreateInfo;
	};
	
	class VKDepthStencilState : public DepthStencilState
	{
	public:
        VKDepthStencilState(const DepthStencilDesc& desc);
        virtual ~VKDepthStencilState() {}

        // get vk create info
        const VkPipelineDepthStencilStateCreateInfo* getVkCreateInfo() { return &m_vkCreateInfo; }

    private:
        VkPipelineDepthStencilStateCreateInfo m_vkCreateInfo;
	};

	class VKRasterizerState : public RasterizerState
	{
	public:
        VKRasterizerState(const RasterizerDesc& desc);
        virtual ~VKRasterizerState(){}

        // get vk create info
        const VkPipelineRasterizationStateCreateInfo* getVkCreateInfo() { return &m_vkCreateInfo; }

    private:
        VkPipelineRasterizationStateCreateInfo m_vkCreateInfo;
	};

	class VKSamplerState : public SamplerState
	{
	public:
        VKSamplerState(const SamplerDesc& desc);
        virtual ~VKSamplerState() {}

        // get vk create info
        const VkPipelineMultisampleStateCreateInfo* getVkCrateInfo() { return &m_vkCreateInfo; }

    private:
        VkPipelineMultisampleStateCreateInfo m_vkCreateInfo;
	};

    class VKMultisampleState : public MultisampleState
    {
    public:
        VKMultisampleState();
        ~VKMultisampleState() {}

        // get vk create info
        const VkPipelineMultisampleStateCreateInfo* getVkCreateInfo() { return &m_vkCreateInfo; }

    private:
        VkPipelineMultisampleStateCreateInfo m_vkCreateInfo;
    };
}
