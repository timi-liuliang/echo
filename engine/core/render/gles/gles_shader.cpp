#include "gles_render_base.h"
#include "gles_shader.h"
#include <engine/core/util/Exception.h>
#include "engine/core/io/IO.h"

namespace Echo
{
	GLESShader::GLESShader(ShaderProgram::ShaderType type, const String& filename)
		: m_filename(filename)
	{
		DataStream* pShaderStream = IO::instance()->open(filename);
		if (!pShaderStream)
		{
			EchoLogError("Shader file isn't exist. [%s]", filename.c_str());
		}

		m_validata = true;
		m_shaderType = type;
		m_shaderSize = static_cast<ui32>(pShaderStream->size() + 1);
		m_srcData.resize(m_shaderSize);
		m_srcData[m_shaderSize - 1] = '\0';

		pShaderStream->read(&m_srcData[0], m_shaderSize);

		EchoSafeDelete(pShaderStream, DataStream);

		replaceInclude();
		create(filename);
	}

	GLESShader::GLESShader(ShaderProgram::ShaderType type, const char* srcBuffer, ui32 size)
		: m_program(NULL)
	{
		m_validata = true;
		m_shaderType = type;
		m_shaderSize = size;

		m_srcData.resize(m_shaderSize);
		memcpy(&m_srcData[0], srcBuffer, size);

		create("");
	}

	GLESShader::~GLESShader()
	{
		if (m_glesShader)
		{
			OGLESDebug(glDeleteShader(m_glesShader));
			m_glesShader = 0;
		}
	}

	void GLESShader::replaceInclude()
	{
		String::size_type pos = m_srcData.find("#include");
		if (pos != String::npos)
		{
			String::size_type begin = String::npos;
			String::size_type end = String::npos;
			for (String::size_type i = pos; i < m_srcData.size(); i++)
			{
				if (m_srcData[i] == '<')
					begin = i;
				if (m_srcData[i] == '>')
					end = i;

				if (begin != String::npos && end != String::npos)
					break;
			}

			if (begin != String::npos && end != String::npos)
			{
				String incName = m_srcData.substr(begin + 1, end - begin - 1);
				if (!incName.empty())
				{
					MemoryReader memReader(incName.c_str());
					if (memReader.getSize())
					{
						m_srcData.replace(pos, end - pos + 1, memReader.getData<char*>());
					}
				}

				replaceInclude();
			}
		}
	}

	void GLESShader::create(const String& filename)
	{
		// Windows platform，Debug mode，enable Nsight Shader debug
#if defined(ECHO_PLATFORM_WINDOWS) && defined(ECHO_DEBUG)
		size_t pos = m_srcData.find_first_of('\n') + 1;
		if (m_shaderType==ShaderProgram::VS)
			m_srcData.insert(pos, "#line 29\r\n");
		else if (m_shaderType==ShaderProgram::FS)
			m_srcData.insert(pos, "#line 30\r\n");
#endif

		switch (m_shaderType)
		{
		case ShaderProgram::VS:
		{
			m_glesShader = OGLESDebug(glCreateShader(GL_VERTEX_SHADER));
			if (!m_glesShader)
				EchoLogError("Create vertex Shader [%s] failed.", filename.c_str());

			break;
		}
		case ShaderProgram::CS:
		{
			m_glesShader = OGLESDebug(glCreateShader(GL_COMPUTE_SHADER));
			if (!m_glesShader)
				EchoLogError("Create computer shader [%s] failed.", filename.c_str());

			break;
		}
		case ShaderProgram::FS:
		{
			m_glesShader = OGLESDebug(glCreateShader(GL_FRAGMENT_SHADER));
			if (!m_glesShader)
				EchoLogError("Create pixel Shader [%s] failed.", filename.c_str());

			break;
		} 
		default:
		{
			EchoLogError("Unknown shader type, create Shader [%s] failed.", filename.c_str());
			break;
		}}

		// bind shader source code to the shader object
		const GLchar* srcData = &m_srcData[0];
		OGLESDebug(glShaderSource(m_glesShader, 1, &srcData, NULL));

		// compile shader.
		OGLESDebug(glCompileShader(m_glesShader));

		// check to see if the vertex shader compiled successfully.
		GLint status;
		OGLESDebug(glGetShaderiv(m_glesShader, GL_COMPILE_STATUS, &status));
		if (status != 1)
		{
			m_validata = false;
			// get the size of the string containing the information log for the failed shader compilation message.
			GLint logSize = 0;
			OGLESDebug(glGetShaderiv(m_glesShader, GL_INFO_LOG_LENGTH, &logSize));

			// increment the size by one to handle also the null terminator.
			++logSize;

			// create a char buffer to hold the info log.
			char* szLogInfo = (char*)EchoMalloc(logSize);

			// retrieve the info log.
			GLint realsize;
			OGLESDebug(glGetShaderInfoLog(m_glesShader, logSize, &realsize, szLogInfo));

			String errMsg = szLogInfo;
			EchoSafeFree(szLogInfo);

			EchoLogError("Compile Shader [%s] Error: \n%s", filename.c_str(), errMsg.c_str());
		}

		m_srcData.clear();
		m_srcData.shrink_to_fit();
	}

	GLuint GLESShader::getShaderHandle() const
	{
		return m_glesShader;
	}

	void GLESShader::setShaderProgram(GLESShaderProgram* pProgram)
	{
		m_program = pProgram;
	}

	GLESShaderProgram* GLESShader::getShaderProgram() const
	{
		return m_program;
	}

	ShaderProgram::ShaderType GLESShader::getShaderType() const
	{
		return m_shaderType;
	}

	String GLESShader::GetShaderTypeDesc(ShaderProgram::ShaderType type)
	{
		switch (type)
		{
		case ShaderProgram::VS:	return "VS";
		case ShaderProgram::CS:	return "CS";
		case ShaderProgram::FS:	return "VS";
		default:				return "Unknown";
		}
	}

	bool GLESShader::validate() const
	{
		return m_validata;
	}
}
