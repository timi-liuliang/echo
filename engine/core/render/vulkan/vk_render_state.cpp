#include "vk_render_state.h"
#include "vk_render_base.h"

namespace Echo
{
    VKBlendState::VKBlendState(const BlendDesc &desc)
        : BlendState(desc)
    {
        VkPipelineColorBlendAttachmentState blendAttachState = {};
        blendAttachState.colorWriteMask = 0xf;

        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_vkCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        m_vkCreateInfo.attachmentCount = 1;
        m_vkCreateInfo.pAttachments = &blendAttachState;
    }

    VKDepthStencilState::VKDepthStencilState(const DepthStencilDesc& desc)
        : DepthStencilState(desc)
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        m_vkCreateInfo.depthTestEnable = VK_TRUE;
        m_vkCreateInfo.depthWriteEnable = VK_TRUE;
        m_vkCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        m_vkCreateInfo.depthBoundsTestEnable = VK_FALSE;
        m_vkCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
        m_vkCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
        m_vkCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        m_vkCreateInfo.stencilTestEnable = VK_FALSE;
        m_vkCreateInfo.front = m_vkCreateInfo.back;
    }

    VKRasterizerState::VKRasterizerState(const RasterizerDesc& desc)
        : RasterizerState(desc)
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_vkCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        m_vkCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        m_vkCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        m_vkCreateInfo.lineWidth = m_desc.lineWidth;
    }

    VKSamplerState::VKSamplerState(const SamplerDesc& desc)
        : SamplerState(desc)
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    }

    VKMultisampleState::VKMultisampleState()
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_vkCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        m_vkCreateInfo.pSampleMask = nullptr;
    }
}