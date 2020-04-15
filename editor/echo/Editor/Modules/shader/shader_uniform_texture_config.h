#pragma once

#include "shader_uniform_config.h"

namespace Echo
{
	class ShaderUniformTextureConfig : public ShaderUniformConfig
	{
		ECHO_CLASS(ShaderUniformTextureConfig, ShaderUniformConfig)

	public:
		ShaderUniformTextureConfig();
		virtual ~ShaderUniformTextureConfig();

	protected:
	};
}