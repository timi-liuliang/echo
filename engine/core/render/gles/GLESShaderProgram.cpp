#include "GLESRenderBase.h"
#include "GLESRenderer.h"
#include "GLESShaderProgram.h"
#include "GLESMapping.h"
#include <engine/core/Util/Exception.h>
#include <engine/core/log/Log.h>
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	GLES2ShaderProgram::GLES2ShaderProgram()
		: ShaderProgram()
	{
		m_attribLocationMapping.assign(-1);

		m_glesProgram = OGLESDebug(glCreateProgram());
		if (!m_glesProgram)
		{
			EchoLogError("Create GLES2ShaderProgram failed.");
		}
		m_preRenderable = NULL;
	}

	GLES2ShaderProgram::~GLES2ShaderProgram()
	{
		(ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(nullptr);

		Shader* pVertexShader = detachShader(Shader::ST_VERTEXSHADER);
		EchoSafeDelete(pVertexShader, Shader);

		Shader* pPixelShader = detachShader(Shader::ST_PIXELSHADER);
		EchoSafeDelete(pPixelShader, Shader);

		OGLESDebug(glDeleteProgram(m_glesProgram));
		m_glesProgram = 0;
	}
	
	bool GLES2ShaderProgram::attachShader(Shader* pShader)
	{
		if (ShaderProgram::attachShader(pShader))
		{
			EchoAssert(pShader);
			GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
			ui32 hShader = pGLES2Shader->getShaderHandle();
			OGLESDebug(glAttachShader(m_glesProgram, hShader));

			return true;
		}
			
		return false;
	}
	
	Shader* GLES2ShaderProgram::detachShader(Shader::ShaderType type)
	{
		Shader* pShader = ShaderProgram::detachShader(type);
		if(pShader)
		{
			GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
			ui32 hShader = pGLES2Shader->getShaderHandle();
			OGLESDebug(glDetachShader(m_glesProgram, hShader));
		}

		return pShader;
	}

	bool GLES2ShaderProgram::linkShaders()
	{
		OGLESDebug(glLinkProgram(m_glesProgram));

		// Check the status of the link.
		GLint status;
		OGLESDebug(glGetProgramiv(m_glesProgram, GL_LINK_STATUS, &status));
		if (status != 1)
		{
			// get the size of the string containing the information log for the failed shader compilation message.
			GLint logSize = 0;
			OGLESDebug(glGetProgramiv(m_glesProgram, GL_INFO_LOG_LENGTH, &logSize));

			// increment the size by one to handle also the null terminator.
			++logSize;

			// create a char buffer to hold the info log.
			char* szLogInfo = (char*)EchoMalloc(logSize);

			// retrieve the info log.
			OGLESDebug(glGetProgramInfoLog(m_glesProgram, logSize, NULL, szLogInfo));

			String errMsg = szLogInfo;
			EchoSafeFree(szLogInfo);

			EchoLogError("Link Shaders Error: \n%s", errMsg.c_str());
			return false;
		}

		m_isLinked = true;
		EchoLogDebug("Link shaders successd.");

		// 记录常量信息
		GLint activeUniformLength;
		OGLESDebug(glGetProgramiv(m_glesProgram, GL_ACTIVE_UNIFORMS, &activeUniformLength));

		// 记录常量信息
		for (GLint i = 0; i < activeUniformLength; i++)
		{
			char    unifromName[512];
			GLint   uniformSize;
			GLenum  uniformType;
			GLsizei uniformLength;
			OGLESDebug(glGetActiveUniform(m_glesProgram, i, 512, &uniformLength, &uniformSize, &uniformType, unifromName));

			Uniform desc;
			desc.m_name = StringUtil::Replace(unifromName, "[0]", "").c_str();
			desc.m_type = GLES2Mapping::MapUniformType(uniformType);
			desc.m_count = uniformSize;
			desc.m_sizeInBytes = desc.m_count * getUniformByteSizeByUniformType(desc.m_type);
			desc.m_location = glGetUniformLocation(m_glesProgram, desc.m_name.c_str());
			m_uniforms[desc.m_location] = desc;
		}

		for (ui32 i = 0; i < VS_MAX; ++i)
		{
			String strName = GLES2Mapping::MapVertexSemanticString((VertexSemantic)i);
			GLint loc = OGLESDebug(glGetAttribLocation(m_glesProgram, strName.c_str()));
			if (loc != -1)
			{
				m_attribLocationMapping[i] = loc;
			}
		}

		m_isLinked = true;

		return true;
	}

	void GLES2ShaderProgram::bindUniforms()
	{
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
				EchoLogError("Shader param is NULL, Material");
			}
		}
	}
	
	void GLES2ShaderProgram::bind()
	{
		if (m_isLinked)
		{
			// Install the shader program as part of the current rendering state.
			if ((ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(this))
			{
				OGLESDebug(glUseProgram(m_glesProgram));

				// set dirty flag
				for (UniformArray::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
				{
					Uniform& uniform = it->second;
					uniform.m_isDirty = true;
				}

				m_preRenderable = NULL;
			}
		}
		else
		{
			EchoLogError("The shader program has not been linked.");
		}
	}

	void GLES2ShaderProgram::bindRenderable(Renderable* renderInput)
	{
		GLES2Renderable* ra = ECHO_DOWN_CAST<GLES2Renderable*>(renderInput);
		ra->bind(m_preRenderable);

		m_preRenderable = ra;
	}
	
	i32 GLES2ShaderProgram::getAtrribLocation(VertexSemantic vertexSemantic)
	{
		return m_attribLocationMapping[vertexSemantic];
	}

	void GLES2ShaderProgram::unbind()
	{
	}

	const String& GLES2ShaderProgram::getMaterialName()
	{
		return "MaterialName:";
	}
}
