#include "GLES2RenderStd.h"
#include "Render/RenderThread.h"
#include "GLES2ShaderTasks.h"
#include "GLES2ShaderGPUProxy.h"

namespace Echo
{

	GLES2ShaderTaskDestroyProxy::GLES2ShaderTaskDestroyProxy(GLES2ShaderGPUProxy* proxy)
		: m_proxy(proxy)
	{
	}

	void GLES2ShaderTaskDestroyProxy::Execute()
	{
		m_proxy->deleteShader();
		EchoSafeDelete(m_proxy, GLES2ShaderGPUProxy);
	}

	GLES2ShaderTaskCreate::GLES2ShaderTaskCreate(GLES2ShaderGPUProxy* proxy, const String& file_name, Shader::ShaderType shader_type, const String& src_data, const Shader::ShaderDesc& desc)
		: m_proxy(proxy)
		, m_file_name(file_name)
		, m_shader_type(shader_type)
		, m_src_data(src_data)
		, m_desc(desc)
	{
	}

	void GLES2ShaderTaskCreate::Execute()
	{
		if (!m_desc.macros.empty())
		{
			m_desc.macros = "\r\n" + m_desc.macros + "\r\n";

			size_t pos = m_src_data.find_first_of('\n') + 1;
			m_src_data.insert(pos, m_desc.macros);
		}
		m_proxy->createShader(m_file_name, m_shader_type, m_src_data);
	}
}
