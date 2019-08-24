#pragma once

#include "engine/core/render/interface/Renderable.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKRenderable : public Renderable
	{
	public:
		VKRenderable(const String& renderStage, ShaderProgram* shader, int identifier);
        virtual ~VKRenderable() {}

        // link shader and program
		virtual void setMesh(Mesh* mesh) override;

		// param operate
		virtual void setShaderParam(const String& name, ShaderParamType type, const void* param, size_t num = 1) {}

	private:
		// create vk pipeline
		void createVkPipeline();

	private:
		VkPipeline          m_vkPipeline;
	};
}
