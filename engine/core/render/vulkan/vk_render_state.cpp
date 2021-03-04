#include "vk_render_state.h"
#include "vk_render_base.h"
#include "vk_mapping.h"

namespace Echo
{
    VKBlendState::VKBlendState(const BlendDesc& desc)
        : BlendState(desc)
    {
        VkPipelineColorBlendAttachmentState blendAttachState = {};
        blendAttachState.blendEnable = desc.bBlendEnable ? VK_TRUE : VK_FALSE;
        blendAttachState.srcColorBlendFactor = VKMapping::mapBlendFactor(desc.srcBlend);
        blendAttachState.dstColorBlendFactor = VKMapping::mapBlendFactor(desc.dstBlend);
        blendAttachState.colorBlendOp = VKMapping::mapBlendOperation(desc.blendOP);
        blendAttachState.srcAlphaBlendFactor = VKMapping::mapBlendFactor(desc.srcAlphaBlend);
        blendAttachState.dstAlphaBlendFactor = VKMapping::mapBlendFactor(desc.dstAlphaBlend);
        blendAttachState.alphaBlendOp = VKMapping::mapBlendOperation(desc.alphaBlendOP);;
        blendAttachState.colorWriteMask = desc.colorWriteMask;

        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_vkCreateInfo.pNext = nullptr;
        m_vkCreateInfo.flags = 0;
        m_vkCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        m_vkCreateInfo.attachmentCount = 1;
        m_vkCreateInfo.pAttachments = &blendAttachState;
        m_vkCreateInfo.blendConstants[0] = desc.blendFactor.r;
        m_vkCreateInfo.blendConstants[1] = desc.blendFactor.g;
        m_vkCreateInfo.blendConstants[2] = desc.blendFactor.b;
        m_vkCreateInfo.blendConstants[3] = desc.blendFactor.a;
    }

    VKDepthStencilState::VKDepthStencilState()
        : DepthStencilState()
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        m_vkCreateInfo.depthTestEnable = bDepthEnable ? VK_TRUE : VK_FALSE;
        m_vkCreateInfo.depthWriteEnable = bWriteDepth ? VK_TRUE : VK_FALSE;
        m_vkCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        m_vkCreateInfo.depthBoundsTestEnable = VK_FALSE;
        m_vkCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
        m_vkCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
        m_vkCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        m_vkCreateInfo.stencilTestEnable = VK_FALSE;
        m_vkCreateInfo.front = m_vkCreateInfo.back;
    }

    VKRasterizerState::VKRasterizerState()
        : RasterizerState()
    {
        m_vkCreateInfo = {};
        m_vkCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_vkCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        m_vkCreateInfo.cullMode = VK_CULL_MODE_NONE;
        m_vkCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        m_vkCreateInfo.lineWidth = m_lineWidth;
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