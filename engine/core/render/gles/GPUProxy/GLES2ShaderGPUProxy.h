#pragma once

#include "Render/Shader.h"

namespace Echo
{
	class GLES2ShaderGPUProxy
	{
	public:
		GLES2ShaderGPUProxy();
		~GLES2ShaderGPUProxy();

		void createShader(const String& filename, Shader::ShaderType shader_type, const String& src_data);
		void deleteShader();

		GLuint			m_hShader;
	};
}