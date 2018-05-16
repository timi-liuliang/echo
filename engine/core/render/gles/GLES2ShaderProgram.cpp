#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2ShaderProgram.h"
#include "GLES2Mapping.h"
#include <engine/core/Util/Exception.h>
#include <engine/core/Util/LogManager.h>
#include "engine/core/Memory/MemManager.h"
#include "Render/Material.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"
#include "Render/UniformCache.h"

#define UNIFORM_OPTIMIZE

namespace Echo
{
	GLES2ShaderProgram::GLES2ShaderProgram( Material* material)
		: ShaderProgram( material)
#ifdef ECHO_RENDER_THREAD
		, m_gpu_proxy(EchoNew(GLES2ShaderProgramGPUProxy))
#endif
	{
		m_attribLocationMapping.assign(-1);

#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2ShaderProgramTaskCreateProgram>::CreateTask(m_gpu_proxy);
#else
		m_hProgram = OGLESDebug(glCreateProgram());
		if (!m_hProgram)
		{
			EchoException("Create GLES2ShaderProgram failed.");
		}
#endif
		m_preRenderInput = NULL;
	}

	GLES2ShaderProgram::~GLES2ShaderProgram()
	{
		(ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(nullptr);

		Shader* pVertexShader = detachShader(Shader::ST_VERTEXSHADER);
		EchoSafeDelete(pVertexShader, Shader);

		Shader* pPixelShader = detachShader(Shader::ST_PIXELSHADER);
		EchoSafeDelete(pPixelShader, Shader);

#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2ShaderProgramTaskDestroyProxy>::CreateTask(m_gpu_proxy);
		m_gpu_proxy = nullptr;
#else
		OGLESDebug(glDeleteProgram(m_hProgram));
		m_hProgram = 0;
#endif
	}
	
	bool GLES2ShaderProgram::attachShader(Shader* pShader)
	{
		bool bRet = ShaderProgram::attachShader(pShader);
		if(!bRet)
			return false;

		EchoAssert(pShader);
		GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2ShaderProgramTaskAttachShader>::CreateTask(m_gpu_proxy, pGLES2Shader->m_proxy);
#else
		ui32 hShader = pGLES2Shader->getShaderHandle();
		OGLESDebug(glAttachShader(m_hProgram, hShader));
#endif

		return true;
	}
	
	Shader* GLES2ShaderProgram::detachShader(Shader::ShaderType type)
	{
		Shader* pShader = ShaderProgram::detachShader(type);
		if(pShader)
		{
			GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
#ifdef ECHO_RENDER_THREAD
			TRenderTask<GLES2ShaderProgramTaskDetachShader>::CreateTask(m_gpu_proxy, pGLES2Shader->m_proxy);
#else
			ui32 hShader = pGLES2Shader->getShaderHandle();
			OGLESDebug(glDetachShader(m_hProgram, hShader));
#endif
		}

		return pShader;
	}

	bool GLES2ShaderProgram::linkShaders()
	{
#ifdef ECHO_RENDER_THREAD
		TRenderTask<GLES2ShaderProgramTaskLinkShaders>::CreateTask(m_gpu_proxy);
		FlushRenderTasks();

		if (m_gpu_proxy->m_link_status != 1)
		{
			return false;
		}

		TRenderTask<GLES2ShaderProgramTaskUniformReflection>::CreateTask(m_gpu_proxy, &m_uniforms);
		FlushRenderTasks();

		TRenderTask<GLES2ShaderProgramTaskAttribReflection>::CreateTask(m_gpu_proxy, &m_attribLocationMapping);
		FlushRenderTasks();
#else
		OGLESDebug(glLinkProgram(m_hProgram));

		// Check the status of the link.
		GLint status;
		OGLESDebug(glGetProgramiv(m_hProgram, GL_LINK_STATUS, &status));
		if (status != 1)
		{
			// get the size of the string containing the information log for the failed shader compilation message.
			GLint logSize = 0;
			OGLESDebug(glGetProgramiv(m_hProgram, GL_INFO_LOG_LENGTH, &logSize));

			// increment the size by one to handle also the null terminator.
			++logSize;

			// create a char buffer to hold the info log.
			char* szLogInfo = (char*)EchoMalloc(logSize);

			// retrieve the info log.
			OGLESDebug(glGetProgramInfoLog(m_hProgram, logSize, NULL, szLogInfo));

			String errMsg = szLogInfo;
			EchoSafeFree(szLogInfo);

			EchoLogError("Link Shaders Error: \n%s", errMsg.c_str());
			return false;
		}

		m_bLinked = true;
		EchoLogDebug("Link shaders successd.");

		// 记录常量信息
		GLint activeUniformLength;
		OGLESDebug(glGetProgramiv(m_hProgram, GL_ACTIVE_UNIFORMS, &activeUniformLength));

		// 记录常量信息
		for (GLint i = 0; i < activeUniformLength; i++)
		{
			char    unifromName[512];
			GLint   uniformSize;
			GLenum  uniformType;
			GLsizei uniformLength;
			OGLESDebug(glGetActiveUniform(m_hProgram, i, 512, &uniformLength, &uniformSize, &uniformType, unifromName));

			Uniform desc;
			desc.m_name = StringUtil::Replace(unifromName, "[0]", "").c_str();
			desc.m_type = GLES2Mapping::MapUniformType(uniformType);
			desc.m_count = uniformSize;
			desc.m_sizeInBytes = desc.m_count * getUniformByteSizeByUniformType(desc.m_type);
			desc.m_location = glGetUniformLocation(m_hProgram, desc.m_name.c_str());
			m_uniforms[desc.m_location] = desc;
		}

		for (ui32 i = 0; i < RenderInput::VS_MAX; ++i)
		{
			String strName = GLES2Mapping::MapVertexSemanticString((RenderInput::VertexSemantic)i);
			GLint loc = OGLESDebug(glGetAttribLocation(m_hProgram, strName.c_str()));
			if (loc != -1)
			{
				m_attribLocationMapping[i] = loc;
			}
		}
#endif

		m_bLinked = true;

		return true;
	}

	// 应用变量
	void GLES2ShaderProgram::bindUniforms()
	{
#ifdef ECHO_RENDER_THREAD

	#ifndef UNIFORM_OPTIMIZE
		TRenderTask<GLES2ShaderProgramTaskBindUniforms>::CreateTask(m_gpu_proxy, m_uniforms);
	#else
		size_t cacheOffset = g_uniform_cache->BeginAppendUniformInfo();

		for (ShaderProgram::UniformArray::const_iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			ShaderProgram::Uniform& uniform = const_cast<ShaderProgram::Uniform&>(it->second);
			if (uniform.m_value)
			{
				if (uniform.m_isDirty && uniform.m_type != SPT_UNKNOWN)
				{
					g_uniform_cache->AppendUniformInfo(uniform.m_type, uniform.m_location, uniform.m_count, uniform.m_origin_value);

					// 修改状态
					uniform.m_isDirty = false;
				}
			}
			else
			{
				EchoLogError("Shader param [%s] is NULL, material name [%s]", uniform.m_name.c_str(), m_material->getName().c_str());
			}
		}

		size_t cacheCount = g_uniform_cache->EndAppendUniformInfo();
		TRenderTask<GLES2ShaderProgramTaskBindUniforms>::CreateTask(m_gpu_proxy, cacheOffset, cacheCount);
	#endif

#else
		for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			Uniform& uniform = it->second;
			if (uniform.m_value)
			{
				if (uniform.m_isDirty && uniform.m_type != SPT_UNKNOWN)
				{
					switch (uniform.m_type)
					{
						case SPT_VEC4:		OGLESDebug(glUniform4fv(uniform.m_location, uniform.m_count, (GLfloat*)uniform.m_value));					break;
						case SPT_MAT4:		OGLESDebug(glUniformMatrix4fv(uniform.m_location, uniform.m_count, false, (GLfloat*)uniform.m_value));		break;
						case SPT_INT:		OGLESDebug(glUniform1iv(uniform.m_location, uniform.m_count, (GLint*)uniform.m_value));					break;
						case SPT_FLOAT:		OGLESDebug(glUniform1fv(uniform.m_location, uniform.m_count, (GLfloat*)uniform.m_value));					break;
						case SPT_VEC2:		OGLESDebug(glUniform2fv(uniform.m_location, uniform.m_count, (GLfloat*)uniform.m_value));					break;
						case SPT_VEC3:		OGLESDebug(glUniform3fv(uniform.m_location, uniform.m_count, (GLfloat*)uniform.m_value));					break;
						case SPT_TEXTURE:	OGLESDebug(glUniform1i(uniform.m_location, *(ui32*)uniform.m_value));										break;
						default:			EchoAssertX(0, "unknow shader param format!");													break;
					}

					// 修改状态
					uniform.m_isDirty = false;
				}
			}
			else
			{
				EchoLogError("Shader param [%s] is NULL, Material [%s]", uniform.m_name.c_str(), m_material->getName().c_str());
			}
		}
#endif
	}
	
	// 绑定着色器
	void GLES2ShaderProgram::bind()
	{
		EchoAssertX(m_bLinked, "The shader program has not been linked.");
	
		// Install the shader program as part of the current rendering state.
		if ((ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(this))
		{
#ifdef ECHO_RENDER_THREAD
			TRenderTask<GLES2ShaderProgramTaskUseProgram>::CreateTask(m_gpu_proxy);
#else
			OGLESDebug(glUseProgram(m_hProgram));
#endif
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
	
	i32 GLES2ShaderProgram::getAtrribLocation(RenderInput::VertexSemantic vertexSemantic)
	{
		return m_attribLocationMapping[vertexSemantic];
	}

	// 解绑着色器
	void GLES2ShaderProgram::unbind()
	{
	}

	const String& GLES2ShaderProgram::getMaterialName()
	{
		return m_material->getName();
	}

}
