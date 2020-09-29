#include "gles_render_base.h"
#include "gles_renderer.h"
#include "gles_shader_program.h"
#include "gles_mapping.h"
#include <engine/core/util/Exception.h>
#include <engine/core/log/Log.h>
#include "engine/core/memory/MemAllocDef.h"

namespace Echo
{
	GLESShaderProgram::GLESShaderProgram()
		: ShaderProgram()
	{
		m_shaders.assign(nullptr);
		m_attribLocationMapping.assign(-1);

		m_preRenderable = NULL;
	}

	GLESShaderProgram::~GLESShaderProgram()
	{
		clearShaderProgram();
	}
	
	bool GLESShaderProgram::attachShader(GLESShader* shader)
	{
		if (shader)
		{
			GLESShader::ShaderType type = shader->getShaderType();
			if (!m_shaders[(ui32)type])
			{
				m_shaders[(ui32)type] = shader;
				shader->setShaderProgram(this);
				m_isLinked = false;

				GLESShader* glesShader = ECHO_DOWN_CAST<GLESShader*>(shader);
				ui32 shaderHandle = glesShader->getShaderHandle();
				OGLESDebug(glAttachShader(m_glesProgram, shaderHandle));

				return true;
			}
		}

		EchoLogError("The shader [%s] has been already attached.", shader ? GLESShader::GetShaderTypeDesc(shader->getShaderType()).c_str() : "");
		return false;
	}
	
	GLESShader* GLESShaderProgram::detachShader(GLESShader::ShaderType type)
	{
		GLESShader* shader = m_shaders[(ui32)type];
		m_shaders[(ui32)type] = nullptr;
		m_isLinked = false;
		if(shader)
		{
			GLESShader* pGLES2Shader = (GLESShader*)shader;
			ui32 hShader = pGLES2Shader->getShaderHandle();
			OGLESDebug(glDetachShader(m_glesProgram, hShader));
		}

		return shader;
	}

	bool GLESShaderProgram::linkShaders()
	{
		m_uniforms.clear();

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
		EchoLogDebug("Link shaders succeed.");

		// remember uniforms info
		GLint activeUniformLength;
		OGLESDebug(glGetProgramiv(m_glesProgram, GL_ACTIVE_UNIFORMS, &activeUniformLength));
		for (GLint i = 0; i < activeUniformLength; i++)
		{
			char    unifromName[512];
			GLint   uniformSize;
			GLenum  uniformType;
			GLsizei uniformLength;
			OGLESDebug(glGetActiveUniform(m_glesProgram, i, 512, &uniformLength, &uniformSize, &uniformType, unifromName));

			String origUniformName = StringUtil::Replace(unifromName, "[0]", "");
			ShaderParamType uniformTypeConvert = GLES2Mapping::MapUniformType(uniformType);

			Uniform* desc = nullptr; 
			if (uniformTypeConvert == SPT_TEXTURE)
			{
				desc = EchoNew(UniformTexture);
			}
			else
			{
				desc = EchoNew(UniformNormal);
			}
			
			desc->m_name = StringUtil::Substr( origUniformName, ".", false);
			desc->m_type = uniformTypeConvert;
			desc->m_count = uniformSize;
			desc->m_sizeInBytes = desc->m_count * mapUniformTypeSize(desc->m_type);
			desc->m_location = glGetUniformLocation(m_glesProgram, origUniformName.c_str());
			m_uniforms[desc->m_name] = desc;
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

	void GLESShaderProgram::bindUniforms()
	{
		for (UniformMap::iterator it = m_uniforms.begin(); it != m_uniforms.end(); it++)
		{
			UniformPtr uniform = it->second;
			void* value = uniform->m_value.empty() ? uniform->getValueDefault().data() : uniform->m_value.data();
			if (value)
			{
				if (uniform->m_type != SPT_UNKNOWN)
				{
					switch (uniform->m_type)
					{
						case SPT_VEC4:		OGLESDebug(glUniform4fv(uniform->m_location, uniform->m_count, (GLfloat*)value));				break;
						case SPT_MAT4:		OGLESDebug(glUniformMatrix4fv(uniform->m_location, uniform->m_count, false, (GLfloat*)value));	break;
						case SPT_INT:		OGLESDebug(glUniform1iv(uniform->m_location, uniform->m_count, (GLint*)value));					break;
						case SPT_FLOAT:		OGLESDebug(glUniform1fv(uniform->m_location, uniform->m_count, (GLfloat*)value));				break;
						case SPT_VEC2:		OGLESDebug(glUniform2fv(uniform->m_location, uniform->m_count, (GLfloat*)value));				break;
						case SPT_VEC3:		OGLESDebug(glUniform3fv(uniform->m_location, uniform->m_count, (GLfloat*)value));				break;
						case SPT_TEXTURE:	OGLESDebug(glUniform1i(uniform->m_location, *(ui32*)value));									break;
						default:			EchoAssertX(0, "unknow shader param format!");													break;
					}
				}
			}
			else
			{
				EchoLogError("Shader param is NULL, Material");
			}
		}
	}
	
	void GLESShaderProgram::bind()
	{
		if (m_isLinked)
		{
			if ((ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance()))->bindShaderProgram(this))
			{
				OGLESDebug(glUseProgram(m_glesProgram));
				m_preRenderable = NULL;
			}
		}
		else
		{
			EchoLogError("The shader program has not been linked.");
		}
	}

	void GLESShaderProgram::bindRenderable(Renderable* renderInput)
	{
		GLESRenderable* ra = ECHO_DOWN_CAST<GLESRenderable*>(renderInput);
		ra->bind(m_preRenderable);

		m_preRenderable = ra;
	}
	
	i32 GLESShaderProgram::getAtrribLocation(VertexSemantic vertexSemantic)
	{
		return m_attribLocationMapping[vertexSemantic];
	}

	void GLESShaderProgram::unbind()
	{
	}

	bool GLESShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
	{
		clearShaderProgram();

		m_glesProgram = OGLESDebug(glCreateProgram());
		if (!m_glesProgram)
		{
			EchoLogError("Create GLES2ShaderProgram failed.");
			return false;
		}

		GLESRenderer* pRenderer = ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance());
		GLESShader*pVertexShader = pRenderer->createShader(GLESShader::ST_VERTEXSHADER, vsContent.data(), (ui32)vsContent.size());
		if (!pVertexShader)
		{
			EchoLogError("Error in create vs file: ");
			return false;
		}

		GLESShader*pPixelShader = pRenderer->createShader(GLESShader::ST_PIXELSHADER, psContent.data(), (ui32)psContent.size());
		if (!pPixelShader)
		{
			EchoLogError("Error in create ps file: ");
			return false;
		}

		// create shader program
		attachShader(pVertexShader);
		attachShader(pPixelShader);
		linkShaders();

		return true;
	}

	void GLESShaderProgram::clearShaderProgram()
	{
		(ECHO_DOWN_CAST<GLESRenderer*>(Renderer::instance()))->bindShaderProgram(nullptr);

		GLESShader* pVertexShader = detachShader(GLESShader::ST_VERTEXSHADER);
		EchoSafeDelete(pVertexShader, GLESShader);

		GLESShader* pPixelShader = detachShader(GLESShader::ST_PIXELSHADER);
		EchoSafeDelete(pPixelShader, GLESShader);

		if (m_glesProgram)
		{
			OGLESDebug(glDeleteProgram(m_glesProgram));
			m_glesProgram = 0;
		}
	}
}
