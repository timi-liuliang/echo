#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2ShaderProgram.h"
#include "GLES2Mapping.h"
#include <Foundation/Util/Exception.h>
#include <Foundation/Util/LogManager.h>
#include "Foundation/Memory/LordMemory.h"
#include "Render/Material.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "GLES2ShaderProgramTasks.h"
#include "GLES2ShaderProgramGPUProxy.h"

namespace LORD
{
	GLES2ShaderProgram::GLES2ShaderProgram( Material* material)
		: ShaderProgram( material)
		, m_gpu_proxy(LordNew(GLES2ShaderProgramGPUProxy))
	{
		TRenderTask<GLES2ShaderProgramTaskCreateProgram>::CreateTask(m_gpu_proxy);
		m_preRenderInput = NULL;
	}

	GLES2ShaderProgram::~GLES2ShaderProgram()
	{
		(LORD_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(nullptr);

		Shader* pVertexShader = detachShader(Shader::ST_VERTEXSHADER);
		LordSafeDelete(pVertexShader);

		Shader* pPixelShader = detachShader(Shader::ST_PIXELSHADER);
		LordSafeDelete(pPixelShader);

		TRenderTask<GLES2ShaderProgramTaskDestroyProxy>::CreateTask(m_gpu_proxy);
		m_gpu_proxy = nullptr;
	}
	
	bool GLES2ShaderProgram::attachShader(Shader* pShader)
	{
		bool bRet = ShaderProgram::attachShader(pShader);
		if(!bRet)
			return false;

		LordAssert(pShader);
		GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
		TRenderTask<GLES2ShaderProgramTaskAttachShader>::CreateTask(m_gpu_proxy, pGLES2Shader->m_proxy);

		return true;
	}
	
	Shader* GLES2ShaderProgram::detachShader(Shader::ShaderType type)
	{
		Shader* pShader = ShaderProgram::detachShader(type);
		if(pShader)
		{
			GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
			TRenderTask<GLES2ShaderProgramTaskDetachShader>::CreateTask(m_gpu_proxy, pGLES2Shader->m_proxy);
		}

		return pShader;
	}

	bool GLES2ShaderProgram::linkShaders()
	{
		TRenderTask<GLES2ShaderProgramTaskLinkShaders>::CreateTask(m_gpu_proxy);
		FlushRenderTasks();

		if (m_gpu_proxy->m_link_status != 1)
		{
			return false;
		}
		
		TRenderTask<GLES2ShaderProgramTaskUniformReflection>::CreateTask(m_gpu_proxy, &m_uniforms);
		FlushRenderTasks();

		m_bLinked = true;
		LordLogDebug("Link shaders successd.");

		return true;
	}

	// 应用变量
	void GLES2ShaderProgram::bindUniforms()
	{
		TRenderTask<GLES2ShaderProgramTaskBindUniforms>::CreateTask(m_gpu_proxy, m_uniforms, m_material->getName());
	}
	
	// 绑定着色器
	void GLES2ShaderProgram::bind()
	{
		LordAssertX(m_bLinked, "The shader program has not been linked.");
	
		// Install the shader program as part of the current rendering state.
		if ((LORD_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(this))
		{
			TRenderTask<GLES2ShaderProgramTaskUseProgram>::CreateTask(m_gpu_proxy);
			// 所有变量设置脏标记
			for( UniformArray::iterator it=m_uniforms.begin(); it!=m_uniforms.end(); it++)
			{
				Uniform& uniform = it->second;
				uniform.m_isDirty = true;
			}

			m_preRenderInput = NULL;
		}
	}

	// 绑定几何体数据
	void GLES2ShaderProgram::bindRenderInput(RenderInput* renderInput)
	{
		renderInput->bind( m_preRenderInput);

		m_preRenderInput = renderInput;
	}
	
	// 解绑着色器
	void GLES2ShaderProgram::unbind()
	{
	}

}
