#pragma once

#include "engine/core/render/interface/Renderable.h"
#include "vk_render_base.h"
#include "vk_shader_program.h"

namespace Echo
{
	class VKRenderable : public Renderable
	{
	public:
		VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
        virtual ~VKRenderable() {}

        // bind shader uniforms
        void bindShaderParams();

    public:
        // get vk pipeline
        VkPipeline getVkPipeline() { return m_vkPipeline; }

    private:
        // link shader and program
        virtual void setMesh(Mesh* mesh) override;

	private:
		// create vk pipeline
		void createVkPipeline();

        // build vertex input attribute
        void buildVkVertexInputAttributeDescriptions(VKShaderProgram* vkShaderProgram, const VertexElementList& vertElements, vector<VkVertexInputAttributeDescription>::type& viAttributeDescriptions);

	private:
		VkPipeline          m_vkPipeline = nullptr;
	};
}
