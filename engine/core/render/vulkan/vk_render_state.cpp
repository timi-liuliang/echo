#include "vk_render_state.h"
#include "vk_render_base.h"
#include "vk_mapping.h"

namespace Echo
{
    VKBlendState::VKBlendState(const BlendDesc& desc)
        : BlendState(desc)
    {
        m_blendAttachState = {};
        m_blendAttachState.blendEnable = desc.bBlendEnable ? VK_TRUE : VK_FALSE;
        m_blendAttachState.srcColorBlendFactor = VKMapping::mapBlendFactor(desc.srcBlend);
        m_blendAttachState.dstColorBlendFactor = VKMapping::mapBlendFactor(desc.dstBlend);
        m_blendAttachState.colorBlendOp = VKMapping::mapBlendOperation(desc.blendOP);
        m_blendAttachState.srcAlphaBlendFactor = VKMapping::mapBlendFactor(desc.srcAlphaBlend);
        m_blendAttachState.dstAlphaBlendFactor = VKMapping::mapBlendFactor(desc.dstAlphaBlend);
        m_blendAttachState.alphaBlendOp = VKMapping::mapBlendOperation(desc.alphaBlendOP);;
        m_blendAttachState.colorWriteMask = desc.colorWriteMask;

        m_vkColorBlendStateCreateInfo = {};
        m_vkColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        m_vkColorBlendStateCreateInfo.pNext = nullptr;
        m_vkColorBlendStateCreateInfo.flags = 0;
        m_vkColorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        m_vkColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_CLEAR;
        m_vkColorBlendStateCreateInfo.attachmentCount = 1;
        m_vkColorBlendStateCreateInfo.pAttachments = &m_blendAttachState;
        m_vkColorBlendStateCreateInfo.blendConstants[0] = desc.blendFactor.r;
        m_vkColorBlendStateCreateInfo.blendConstants[1] = desc.blendFactor.g;
        m_vkColorBlendStateCreateInfo.blendConstants[2] = desc.blendFactor.b;
        m_vkColorBlendStateCreateInfo.blendConstants[3] = desc.blendFactor.a;
    }

    VKDepthStencilState::VKDepthStencilState()
        : DepthStencilState()
    {
        m_vkDepthStencilStateCreateInfo = {};
        m_vkDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        m_vkDepthStencilStateCreateInfo.depthTestEnable = bDepthEnable ? VK_TRUE : VK_FALSE;
        m_vkDepthStencilStateCreateInfo.depthWriteEnable = bWriteDepth ? VK_TRUE : VK_FALSE;
        m_vkDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS;
        m_vkDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        m_vkDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
        m_vkDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
        m_vkDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        m_vkDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        m_vkDepthStencilStateCreateInfo.front = m_vkDepthStencilStateCreateInfo.back;
    }

    VKRasterizerState::VKRasterizerState()
        : RasterizerState()
    {
        m_vkRasterizationStateCreateInfo = {};
        m_vkRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_vkRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        m_vkRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
        m_vkRasterizationStateCreateInfo.frontFace = isFrontFaceCCW() ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
        m_vkRasterizationStateCreateInfo.lineWidth = m_lineWidth;
    }

    VKSamplerState::VKSamplerState(const SamplerDesc& desc)
        : SamplerState(desc)
    {
        m_vkSampleStateCreateInfo = {};
        m_vkSampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_vkSampleStateCreateInfo.pNext = nullptr;
        m_vkSampleStateCreateInfo.flags = 0;
        m_vkSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
        m_vkSampleStateCreateInfo.sampleShadingEnable = false;
        m_vkSampleStateCreateInfo.minSampleShading = 0.f;
        m_vkSampleStateCreateInfo.pSampleMask = nullptr;
        m_vkSampleStateCreateInfo.alphaToCoverageEnable = false;
        m_vkSampleStateCreateInfo.alphaToOneEnable = false;
    }

    VKMultisampleState::VKMultisampleState()
    {
        m_vkMultiSampleStateCreateInfo = {};
        m_vkMultiSampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_vkMultiSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        m_vkMultiSampleStateCreateInfo.pSampleMask = nullptr;
    }
}