#pragma once

#include "engine/core/render/base/render_state.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKBlendState : public BlendState
	{
	public:
        VKBlendState(const BlendDesc &desc);
        virtual ~VKBlendState(){}

        // get vk create info
        const VkPipelineColorBlendStateCreateInfo* getVkCreateInfo() { return &m_vkColorBlendStateCreateInfo; }

    private:
        VkPipelineColorBlendAttachmentState     m_blendAttachState;
        VkPipelineColorBlendStateCreateInfo     m_vkColorBlendStateCreateInfo;
	};
	
	class VKDepthStencilState : public DepthStencilState
	{
	public:
        VKDepthStencilState();
        virtual ~VKDepthStencilState() {}

        // get vk create info
        const VkPipelineDepthStencilStateCreateInfo* getVkCreateInfo() { return &m_vkDepthStencilStateCreateInfo; }

    private:
        VkPipelineDepthStencilStateCreateInfo m_vkDepthStencilStateCreateInfo;
	};

	class VKRasterizerState : public RasterizerState
	{
	public:
        VKRasterizerState();
        virtual ~VKRasterizerState(){}

        // get vk create info
        const VkPipelineRasterizationStateCreateInfo* getVkCreateInfo() { return &m_vkRasterizationStateCreateInfo; }

    private:
        VkPipelineRasterizationStateCreateInfo m_vkRasterizationStateCreateInfo;
	};

	class VKSamplerState : public SamplerState
	{
	public:
        VKSamplerState(const SamplerDesc& desc);
        virtual ~VKSamplerState();

        // get vk create info
        const VkSampler getVkSampler() { return m_vkSampler; }

    private:
        VkSamplerCreateInfo m_vkSamplerCreateInfo;
        VkSampler           m_vkSampler = VK_NULL_HANDLE;
	};

    class VKMultisampleState : public MultisampleState
    {
    public:
        VKMultisampleState();
        ~VKMultisampleState() {}

        // get vk create info
        const VkPipelineMultisampleStateCreateInfo* getVkCreateInfo() { return &m_vkMultiSampleStateCreateInfo; }

    private:
        VkPipelineMultisampleStateCreateInfo m_vkMultiSampleStateCreateInfo;
    };
}
