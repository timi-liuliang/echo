#include "vk_render_state.h"
#include "vk_render_base.h"
#include "vk_mapping.h"
#include "vk_renderer.h"

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
        // vulkan use different clockwise, because it has special coordinate-system http://anki3d.org/vulkan-coordinate-system/
        m_vkRasterizationStateCreateInfo = {};
        m_vkRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        m_vkRasterizationStateCreateInfo.polygonMode = VKMapping::mapPolygonMode(m_polygonMode);
        m_vkRasterizationStateCreateInfo.cullMode = VKMapping::mapCullMode(m_cullMode);
        m_vkRasterizationStateCreateInfo.frontFace = isFrontFaceCCW() ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        m_vkRasterizationStateCreateInfo.lineWidth = m_lineWidth;
    }

    VKSamplerState::VKSamplerState(const SamplerDesc& desc)
        : SamplerState(desc)
    {
        m_vkSamplerCreateInfo = {};
        m_vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        m_vkSamplerCreateInfo.pNext = nullptr;
        m_vkSamplerCreateInfo.flags = 0;
        m_vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        m_vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        m_vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        m_vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        m_vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        m_vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        m_vkSamplerCreateInfo.mipLodBias = 0.f;
        m_vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        m_vkSamplerCreateInfo.minLod = 0.f;
        m_vkSamplerCreateInfo.maxLod = 0.f;
        m_vkSamplerCreateInfo.maxAnisotropy = 1.f;
        m_vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        
        VKDebug(vkCreateSampler(VKRenderer::instance()->getVkDevice(), &m_vkSamplerCreateInfo, nullptr, &m_vkSampler));
    }

    VKSamplerState::~VKSamplerState()
    {
        vkDestroySampler(VKRenderer::instance()->getVkDevice(), m_vkSampler, nullptr);
    }

    VKMultisampleState::VKMultisampleState()
    {
        m_vkMultiSampleStateCreateInfo = {};
        m_vkMultiSampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        m_vkMultiSampleStateCreateInfo.pNext = nullptr;
        m_vkMultiSampleStateCreateInfo.flags = 0;
        m_vkMultiSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        m_vkMultiSampleStateCreateInfo.sampleShadingEnable = false;
        m_vkMultiSampleStateCreateInfo.minSampleShading = 0.f;
        m_vkMultiSampleStateCreateInfo.pSampleMask = nullptr;
        m_vkMultiSampleStateCreateInfo.alphaToCoverageEnable = false;
        m_vkMultiSampleStateCreateInfo.alphaToOneEnable = false;
    }
}