#pragma once

#include "base/Renderable.h"
#include "vk_render_base.h"
#include "vk_shader_program.h"

namespace Echo
{
	class VKRenderable : public Renderable
	{
	public:
		VKRenderable(int identifier);
        virtual ~VKRenderable() {}

        // bind shader uniforms
		void bindRenderState();
        void bindShaderParams(VkCommandBuffer& vkCommandbuffer);
        void bindGeometry(VkCommandBuffer& vkCommandbuffer);

        // create|destroy vk pipeline
        bool createVkPipeline(class VKFramebuffer* vkFrameBuffer);
        void destroyVkPipeline();

    public:
        // get vk pipeline
        VkPipeline getVkPipeline() { return m_vkPipeline; }

    private:
        // link shader and program
        virtual void setMesh(MeshPtr mesh) override;

	private:
        // build vertex input attribute
        void buildVkVertexInputAttributeDescriptions(VKShaderProgram* vkShaderProgram, const VertexElementList& vertElements, vector<VkVertexInputAttributeDescription>::type& viAttributeDescriptions);

        // get vertex attribute by semantic
        bool getVkVertexAttributeBySemantic(VertexSemantic semantic, spirv_cross::Resource& oResource);

        // get blend state create info
        const VkPipelineColorBlendStateCreateInfo* getVkColorBlendStateCreateInfo();

        // get rasterization state create info
        const VkPipelineRasterizationStateCreateInfo* getVkRasterizationStateCreateInfo();

        // get depth stencil state create info
        const VkPipelineDepthStencilStateCreateInfo* getVkDepthStencilStateCrateInfo();

        // get multisample state create creteinfo
        const VkPipelineMultisampleStateCreateInfo* getVkMultiSampleStateCreateInfo();

	private:
        VkGraphicsPipelineCreateInfo        m_vkPipelineInfo = {};
		VkPipeline                          m_vkPipeline = VK_NULL_HANDLE;
        VKShaderProgram::UniformsInstance   m_vkUniformsInstance;
	};
}
