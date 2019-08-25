#include "vk_renderable.h"
#include "vk_renderer.h"
#include "vk_mapping.h"
#include "vk_shader_program.h"

namespace Echo
{
    VKRenderable::VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {
    }

    void VKRenderable::setMesh(Mesh* mesh)
    {
        m_mesh = mesh;

        createVkPipeline();
    }

    void VKRenderable::createVkPipeline()
    {
        if (m_mesh && m_shaderProgram && VKFramebuffer::current())
        {
            VKFramebuffer* vkFrameBuffer = VKFramebuffer::current();
            VKShaderProgram* vkShaderProgram = ECHO_DOWN_CAST<VKShaderProgram*>(m_shaderProgram.ptr());

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
            vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = VKMapping::MapPrimitiveTopology(m_mesh->getTopologyType());

            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.pNext = nullptr;
            pipelineInfo.stageCount = vkShaderProgram->getVkShaderStageCreateInfo().size();
            pipelineInfo.pStages = vkShaderProgram->getVkShaderStageCreateInfo().data();
            pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
            pipelineInfo.pViewportState = vkFrameBuffer->getVkViewportStateCreateInfo();
            pipelineInfo.pDepthStencilState = m_depthStencilState ? nullptr : nullptr;
            pipelineInfo.pRasterizationState = m_rasterizerState ? nullptr : nullptr;
            pipelineInfo.pMultisampleState = m_multiSampleState ? nullptr : nullptr;
            pipelineInfo.pColorBlendState = m_blendState ? nullptr : nullptr;
            pipelineInfo.renderPass = vkFrameBuffer->getVkRenderPass();
            pipelineInfo.basePipelineIndex = -1;
            if (VK_SUCCESS != vkCreateGraphicsPipelines(VKRenderer::instance()->getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkPipeline))
            {
                EchoLogError("vulkan create pipeline failed");
            }
        }
    }
}
