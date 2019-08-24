#include "vk_render_state.h"
#include "vk_render_base.h"

namespace Echo
{
    void VKBlendState::active()
    {
        VkPipelineColorBlendAttachmentState blendAttachState = {};
        blendAttachState.colorWriteMask = 0xf;

        VkPipelineColorBlendStateCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        createInfo.logicOp = VK_LOGIC_OP_COPY;
        createInfo.attachmentCount = 1;
        createInfo.pAttachments = &blendAttachState;
    }

    void VKRasterizerState::active()
    {
        VkPipelineRasterizationStateCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        createInfo.polygonMode = VK_POLYGON_MODE_FILL;
        createInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        createInfo.lineWidth = m_desc.lineWidth;
    }

    void VKSamplerState::active(const SamplerState* pre) const
    {
        VkPipelineMultisampleStateCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    }
}