#include "GLES2RenderStd.h"
#include "GLES2Shader.h"
#include <Foundation/Util/Exception.h>
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GPUProxy/GLES2ShaderGPUProxy.h"
#include "GLES2ShaderTasks.h"


namespace LORD
{
	GLES2Shader::GLES2Shader(ShaderType type, const ShaderDesc& desc, const String& filename)
		: Shader(type, desc, filename)
	{
		m_proxy = LordNew(GLES2ShaderGPUProxy);
		create(filename);
	}

	GLES2Shader::GLES2Shader(ShaderType type, const ShaderDesc& desc, const Buffer& buff)
		: Shader(type, desc, buff)
	{
		m_proxy = LordNew(GLES2ShaderGPUProxy);
		create("");
	}

	GLES2Shader::~GLES2Shader()
	{
		TRenderTask<GLES2ShaderTaskDestroyProxy>::CreateTask(m_proxy);
	}

	void GLES2Shader::create(const String& filename)
	{
		TRenderTask<GLES2ShaderTaskCreate>::CreateTask(m_proxy, filename, m_shaderType, m_srcData);
		m_srcData.clear();
	}



}
