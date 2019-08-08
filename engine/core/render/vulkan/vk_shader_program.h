#pragma once

#include "engine/core/render/interface/shaderprogram.h"
#include "vk_render_base.h"

namespace Echo
{
	class VKShaderProgram : public ShaderProgram
	{
    public:
		virtual ~VKShaderProgram();

	private:
		// create shader library
		virtual bool createShaderProgram(const String& vsContent, const String& psContent) override;

	private:
		bool			m_isValid = false;
		VkShaderModule	m_vkVertexShader;
		VkShaderModule	m_vkFragmentShader;
	};
}
