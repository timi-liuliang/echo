#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "GLES2ShaderTasks.h"
#include "GLES2ShaderGPUProxy.h"

namespace LORD
{

	GLES2ShaderTaskDestroyProxy::GLES2ShaderTaskDestroyProxy(GLES2ShaderGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}

	void GLES2ShaderTaskDestroyProxy::Execute()
	{
		m_proxy->deleteShader();
		LordDelete(m_proxy);
	}

	GLES2ShaderTaskCreate::GLES2ShaderTaskCreate(GLES2ShaderGPUProxy* proxy, const String& file_name, Shader::ShaderType shader_type, const String& src_data)
		: m_proxy(proxy)
		, m_file_name(file_name)
		, m_shader_type(shader_type)
		, m_src_data(src_data)
	{
	}

	void GLES2ShaderTaskCreate::Execute()
	{
		m_proxy->createShader(m_file_name, m_shader_type, m_src_data);
	}

}