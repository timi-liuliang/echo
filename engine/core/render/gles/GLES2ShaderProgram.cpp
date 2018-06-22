#include "GLES2RenderStd.h"
#include "GLES2Renderer.h"
#include "GLES2ShaderProgram.h"
#include "GLES2Mapping.h"
#include <engine/core/Util/Exception.h>
#include <engine/core/log/LogManager.h>
#include "engine/core/Memory/MemManager.h"
#include "Render/ShaderProgramRes.h"

#define UNIFORM_OPTIMIZE

namespace Echo
{
	GLES2ShaderProgram::GLES2ShaderProgram( ShaderProgramRes* material)
		: ShaderProgram( material)
	{
		m_attribLocationMapping.assign(-1);

		m_hProgram = OGLESDebug(glCreateProgram());
		if (!m_hProgram)
		{
			EchoLogError("Create GLES2ShaderProgram failed.");
		}
		m_preRenderInput = NULL;
	}

	GLES2ShaderProgram::~GLES2ShaderProgram()
	{
		(ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(nullptr);

		Shader* pVertexShader = detachShader(Shader::ST_VERTEXSHADER);
		EchoSafeDelete(pVertexShader, Shader);

		Shader* pPixelShader = detachShader(Shader::ST_PIXELSHADER);
		EchoSafeDelete(pPixelShader, Shader);

		OGLESDebug(glDeleteProgram(m_hProgram));
		m_hProgram = 0;
	}
	
	bool GLES2ShaderProgram::attachShader(Shader* pShader)
	{
		bool bRet = ShaderProgram::attachShader(pShader);
		if(!bRet)
			return false;

		EchoAssert(pShader);
		GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
		ui32 hShader = pGLES2Shader->getShaderHandle();
		OGLESDebug(glAttachShader(m_hProgram, hShader));

		return true;
	}
	
	Shader* GLES2ShaderProgram::detachShader(Shader::ShaderType type)
	{
		Shader* pShader = ShaderProgram::detachShader(type);
		if(pShader)
		{
			GLES2Shader* pGLES2Shader = (GLES2Shader*)pShader;
			ui32 hShader = pGLES2Shader->getShaderHandle();
			OGLESDebug(glDetachShader(m_hProgram, hShader));
		}

		return pShader;
	}

	bool GLES2ShaderProgram::linkShaders()
	{
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

		m_bLinked = true;

		return true;
	}

	// 应用变量
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
				EchoLogError("Shader param [%s] is NULL, Material [%s]", uniform.m_name.c_str(), m_material->getName().c_str());
			}
		}
	}
	
	// 绑定着色器
	void GLES2ShaderProgram::bind()
	{
		EchoAssertX(m_bLinked, "The shader program has not been linked.");
	
		// Install the shader program as part of the current rendering state.
		if ((ECHO_DOWN_CAST<GLES2Renderer*>(Renderer::instance()))->bindShaderProgram(this))
		{
			OGLESDebug(glUseProgram(m_hProgram));

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
