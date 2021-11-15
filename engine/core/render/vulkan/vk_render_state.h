#pragma once

#include "base/state/render_state.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKBlendState : public BlendState
	{
	public:
        VKBlendState();
        virtual ~VKBlendState(){}

        // get vk create info
        const VkPipelineColorBlendStateCreateInfo* getVkCreateInfo();

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
        const VkPipelineDepthStencilStateCreateInfo* getVkCreateInfo();

    private:
        VkPipelineDepthStencilStateCreateInfo m_vkDepthStencilStateCreateInfo;
	};

	class VKRasterizerState : public RasterizerState
	{
	public:
        VKRasterizerState();
        virtual ~VKRasterizerState(){}

        // get vk create info
        const VkPipelineRasterizationStateCreateInfo* getVkCreateInfo();

    private:
        VkPipelineRasterizationStateCreateInfo m_vkRasterizationStateCreateInfo;
	};

	class VKSamplerState : public SamplerState
	{
	public:
        VKSamplerState();
        virtual ~VKSamplerState();

        // get vk create info
        const VkSampler getVkSampler();

    private:
        VkSamplerCreateInfo m_vkSamplerCreateInfo;
        VkSampler           m_vkSampler = VK_NULL_HANDLE;
	};
    typedef ResRef<SamplerState> VKSamplerStatePtr;

    class VKMultisampleState : public MultisampleState
    {
    public:
        VKMultisampleState();
        ~VKMultisampleState() {}

        // get vk create info
        const VkPipelineMultisampleStateCreateInfo* getVkCreateInfo();

    private:
        VkPipelineMultisampleStateCreateInfo m_vkMultiSampleStateCreateInfo;
    };
}
