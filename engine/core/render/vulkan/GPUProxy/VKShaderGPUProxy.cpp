#include "GLES2RenderStd.h"
#include "GLES2Shader.h"
#include "engine/core/Util/Exception.h"
#include "GLES2ShaderGPUProxy.h"


namespace LORD
{
	GLES2ShaderGPUProxy::GLES2ShaderGPUProxy()
	{
	}

	GLES2ShaderGPUProxy::~GLES2ShaderGPUProxy()
	{
	}

	void GLES2ShaderGPUProxy::createShader(const String& filename, Shader::ShaderType shader_type, const String& src_data)
	{
		switch (shader_type)
		{
		case Shader::ST_VERTEXSHADER:
		{
			m_hShader = OGLESDebug(glCreateShader(GL_VERTEX_SHADER));
			if (!m_hShader)
			{
				if (filename.empty())
					LordException("Create vertex Shader failed.");
				else
					LordException("Create vertex Shader [%s] failed.", filename.c_str());
			}
		}
		break;
		case Shader::ST_PIXELSHADER:
		{
			m_hShader = OGLESDebug(glCreateShader(GL_FRAGMENT_SHADER));
			if (!m_hShader)
			{
				if (filename.empty())
					LordException("Create pixel Shader failed.");
				else
					LordException("Create pixel Shader [%s] failed.", filename.c_str());
			}
		}
		break;
		default:
		{
			if (filename.empty())
				LordException("Unknown shader type, create Shader failed.");
			else
				LordException("Unknown shader type, create Shader [%s] failed.", filename.c_str());
		}
		break;
		}

		// bind shader source code to the shader object
		const GLchar* srcData = &src_data[0];
		OGLESDebug(glShaderSource(m_hShader, 1, &srcData, NULL));

		// compile shader.
		OGLESDebug(glCompileShader(m_hShader));

		// check to see if the vertex shader compiled successfully.
		GLint status;
		OGLESDebug(glGetShaderiv(m_hShader, GL_COMPILE_STATUS, &status));
		if (status != 1)
		{
			// get the size of the string containing the information log for the failed shader compilation message.
			GLint logSize = 0;
			OGLESDebug(glGetShaderiv(m_hShader, GL_INFO_LOG_LENGTH, &logSize));

			// increment the size by one to handle also the null terminator.
			++logSize;

			// create a char buffer to hold the info log.
			char* szLogInfo = (char*)LordMalloc(logSize);

			// retrieve the info log.
			GLint realsize;
			OGLESDebug(glGetShaderInfoLog(m_hShader, logSize, &realsize, szLogInfo));

			if (filename.empty())
				LordException("Compile Shader Error: \n%s", szLogInfo);
			else
				LordException("Compile Shader [%s] Error: \n%s", filename.c_str(), szLogInfo);

			LordSafeFree(szLogInfo);
		}

	}

	void GLES2ShaderGPUProxy::deleteShader()
	{
		if (m_hShader)
		{
			OGLESDebug(glDeleteShader(m_hShader));
			m_hShader = 0;
		}
	}
}