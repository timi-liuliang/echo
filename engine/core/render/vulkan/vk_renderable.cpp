#include "vk_renderable.h"
#include "vk_renderer.h"
#include "vk_mapping.h"
#include "vk_shader_program.h"
#include "vk_render_state.h"
#include "engine/core/scene/render_node.h"

namespace Echo
{
    VKRenderable::VKRenderable(int identifier)
        : Renderable( identifier)
    {
    }

    void VKRenderable::setMesh(Mesh* mesh)
    {
        m_mesh = mesh;

        createVkPipeline();
    }

    void VKRenderable::createVkPipeline()
    {
        destroyVkPipeline();

        VKShaderProgram* vkShaderProgram = ECHO_DOWN_CAST<VKShaderProgram*>(m_material->getShader());
        if (m_mesh && vkShaderProgram && vkShaderProgram->isLinked() && VKFramebuffer::current())
        {
            VKFramebuffer* vkFrameBuffer = VKFramebuffer::current();

            VkVertexInputBindingDescription vertexInputBinding = {};
            vertexInputBinding.binding = 0;
            vertexInputBinding.stride = m_mesh->getVertexStride();
            vertexInputBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            vector<VkVertexInputAttributeDescription>::type viAttributeDescriptions;
            buildVkVertexInputAttributeDescriptions(vkShaderProgram, m_mesh->getVertexElements(), viAttributeDescriptions);

            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
            vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &vertexInputBinding;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = viAttributeDescriptions.size();
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = viAttributeDescriptions.data();

            VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
            pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            pipelineInputAssemblyStateCreateInfo.topology = VKMapping::MapPrimitiveTopology(m_mesh->getTopologyType());

            vector<VkDynamicState>::type dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
            VkPipelineDynamicStateCreateInfo dynamicState = {};
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.dynamicStateCount = dynamicStateEnables.size();
            dynamicState.pDynamicStates = dynamicStateEnables.data();

            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.layout = vkShaderProgram->getVkPipelineLayout();
            pipelineInfo.renderPass = vkFrameBuffer->getVkRenderPass();
            pipelineInfo.stageCount = vkShaderProgram->getVkShaderStageCreateInfo().size();
            pipelineInfo.pStages = vkShaderProgram->getVkShaderStageCreateInfo().data();
            pipelineInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
            pipelineInfo.pViewportState = vkFrameBuffer->getVkViewportStateCreateInfo();
            pipelineInfo.pDepthStencilState = getVkDepthStencilStateCrateInfo();
            pipelineInfo.pRasterizationState = getVkRasterizationStateCreateInfo();
            pipelineInfo.pMultisampleState = getVkMultiSampleStateCreateInfo();
            pipelineInfo.pColorBlendState = getVkColorBlendStateCreateInfo();
            pipelineInfo.pDynamicState = &dynamicState;

            VKDebug(vkCreateGraphicsPipelines(VKRenderer::instance()->getVkDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vkPipeline));
        }
    }

    void VKRenderable::destroyVkPipeline()
    {
        if (m_vkPipeline)
        {
            vkDestroyPipeline(VKRenderer::instance()->getVkDevice(), m_vkPipeline, nullptr);
            m_vkPipeline = nullptr;
        }
    }

    bool VKRenderable::getVkVertexAttributeBySemantic(VertexSemantic semantic, spirv_cross::Resource& oResource)
    {
        String attributeName = VKMapping::MapVertexSemanticString(semantic);
        VKShaderProgram* vkShaderProgram = ECHO_DOWN_CAST<VKShaderProgram*>(m_material->getShader());
        if (vkShaderProgram && vkShaderProgram->isLinked())
        {
            spirv_cross::ShaderResources vertexShaderResources = vkShaderProgram->getSpirvShaderResources(ShaderProgram::VS);
            for (auto& resource : vertexShaderResources.stage_inputs)
            {
                if (resource.name == attributeName)
                {
                    oResource = resource;
                    return true;
                }
            }
        }

        return false;
    }

    void VKRenderable::buildVkVertexInputAttributeDescriptions(VKShaderProgram* vkShaderProgram, const VertexElementList& vertElements, vector<VkVertexInputAttributeDescription>::type& viAttributeDescriptions)
    {
        if (!vertElements.empty())
        {
            const i32 bufferIdx = 1;
            ui32 elementOffset = 0;
            for (size_t i = 0; i < vertElements.size(); i++)
            {
                spirv_cross::Resource spirvResource = {};
                if (getVkVertexAttributeBySemantic(vertElements[i].m_semantic, spirvResource))
                {
                    const spirv_cross::Compiler* compiler = vkShaderProgram->getSpirvShaderCompiler(ShaderProgram::VS);

                    VkVertexInputAttributeDescription attributeDescription;
                    attributeDescription.binding = compiler->get_decoration(spirvResource.id, spv::DecorationBinding);
                    attributeDescription.location = compiler->get_decoration(spirvResource.id, spv::DecorationLocation);
                    attributeDescription.format = VKMapping::MapVertexFormat(vertElements[i].m_pixFmt);
                    attributeDescription.offset = elementOffset;
                    viAttributeDescriptions.push_back(attributeDescription);
                }

                elementOffset += PixelUtil::GetPixelSize(vertElements[i].m_pixFmt);
            }
        }
    }

    void VKRenderable::bindShaderParams()
    {
		VKShaderProgram* vkShaderProgram = ECHO_DOWN_CAST<VKShaderProgram*>(m_material->getShader());
		if (vkShaderProgram)
		{
			for (auto& it : vkShaderProgram->getUniforms())
			{
				ShaderProgram::UniformPtr uniform = it.second;
				if (uniform->m_type != SPT_TEXTURE)
				{
					void* value = m_node ? m_node->getGlobalUniformValue(uniform->m_name) : nullptr;
					if (!value) value = m_material->getUniformValue(uniform->m_name);

                    vkShaderProgram->setUniform(uniform->m_name.c_str(), value, uniform->m_type, uniform->m_count);
				}
				else
				{
					i32* slotIdxPtr = (i32*)m_material->getUniformValue(uniform->m_name);
					Texture* texture = m_material->getTexture(*slotIdxPtr);
					if (texture)
					{
						Renderer::instance()->setTexture(*slotIdxPtr, texture);
					}

                    vkShaderProgram->setUniform(uniform->m_name.c_str(), slotIdxPtr, uniform->m_type, uniform->m_count);
				}
			}

			vkShaderProgram->bindUniforms();
		}
    }

    void VKRenderable::bindGeometry()
    {
        VKBuffer* vertexBuffer = ECHO_DOWN_CAST<VKBuffer*>(m_mesh->getVertexBuffer());
        if (vertexBuffer)
        {
            VkDeviceSize offsets[1] = { 0 };
            VkBuffer vkBuffer = vertexBuffer->getVkBuffer();
            vkCmdBindVertexBuffers(VKFramebuffer::current()->getVkCommandbuffer(), 0, 1, &vkBuffer, offsets);
        }

        VKBuffer* indexBuffer = ECHO_DOWN_CAST<VKBuffer*>(m_mesh->getIndexBuffer());
        if (indexBuffer)
        {
            vkCmdBindIndexBuffer(VKFramebuffer::current()->getVkCommandbuffer(), indexBuffer->getVkBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    const VkPipelineColorBlendStateCreateInfo* VKRenderable::getVkColorBlendStateCreateInfo()
    {
        VKBlendState* vkState = ECHO_DOWN_CAST<VKBlendState*>( m_material->getShader()->getBlendState());
        return vkState->getVkCreateInfo();
    }

    const VkPipelineRasterizationStateCreateInfo* VKRenderable::getVkRasterizationStateCreateInfo()
    {
        VKRasterizerState* vkState = ECHO_DOWN_CAST<VKRasterizerState*>(m_material->getShader()->getRasterizerState());
        return vkState->getVkCreateInfo();
    }

    const VkPipelineDepthStencilStateCreateInfo* VKRenderable::getVkDepthStencilStateCrateInfo()
    {
        VKDepthStencilState* vkState = ECHO_DOWN_CAST<VKDepthStencilState*>(m_material->getShader()->getDepthState());
        return vkState->getVkCreateInfo();
    }

    const VkPipelineMultisampleStateCreateInfo* VKRenderable::getVkMultiSampleStateCreateInfo()
    {
        VKMultisampleState* vkState = ECHO_DOWN_CAST<VKMultisampleState*>(m_material->getShader()->getMultisampleState());
        return vkState->getVkCreateInfo();
    }
}
