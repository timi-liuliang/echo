#pragma once
#include "GLES2Shader.h"
#include "GLES2ShaderProgram.h"

namespace Echo
{
	class GLES2ShaderTaskDestroyProxy
	{
	public:
		GLES2ShaderTaskDestroyProxy(GLES2ShaderGPUProxy* proxy);
		void Execute();
	private:
		GLES2ShaderGPUProxy* m_proxy;
	};

	class GLES2ShaderTaskCreate
	{
	public:
		GLES2ShaderTaskCreate(GLES2ShaderGPUProxy* proxy, const String& file_name, Shader::ShaderType shader_type, const String& src_data, const Shader::ShaderDesc& desc);
		void Execute();

	private:
		GLES2ShaderGPUProxy* m_proxy;
		String m_file_name;
		Shader::ShaderType m_shader_type;
		String m_src_data;
		Shader::ShaderDesc m_desc;
	};
}