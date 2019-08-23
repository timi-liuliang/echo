#include "vk_renderable.h"

namespace Echo
{
    VKRenderable::VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier)
        : Renderable( renderStage, shader, identifier)
    {

    }

    void VKRenderable::setMesh(Mesh* mesh)
    {
        m_mesh = mesh;
    }

    void VKRenderable::createVkPipeline()
    {
        VKRenderer* vkRenderer = ECHO_DOWN_CAST<VKRenderer*>(Renderer::instance());

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = ARRAY_SIZE_IN_ELEMENTS(shaderStageCreateInfo);
        pipelineInfo.pStages = &shaderStageCreateInfo[0];
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineIACreateInfo;
        pipelineInfo.pViewportState = &vpCreateInfo;
        pipelineInfo.pDepthStencilState = &dsInfo;
        pipelineInfo.pRasterizationState = &rastCreateInfo;
        pipelineInfo.pMultisampleState = &pipelineMSCreateInfo;
        pipelineInfo.pColorBlendState = &blendCreateInfo;
        pipelineInfo.renderPass = m_renderPass;
        pipelineInfo.basePipelineIndex = -1;
        if(VK_SUCCESS != vkCreateGraphicsPipelines(vkRenderer->getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkPipeline))
        {
            EchoLogError("vulkan create pipeline failed");
        }
    }
}
