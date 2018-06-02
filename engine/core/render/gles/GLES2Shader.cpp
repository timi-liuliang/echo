#include "GLES2RenderStd.h"
#include "GLES2Shader.h"
#include <engine/core/Util/Exception.h>
#include "engine/core/io/DataStream.h"
#include "Render/RenderThread.h"
#include "Render/RenderTask.h"

namespace Echo
{
	GLES2Shader::GLES2Shader(ShaderType type, const ShaderDesc& desc, const String& filename)
		: Shader(type, desc, filename)
	{
		// 替换include文件
		replaceInclude();
		create(filename);
	}

	GLES2Shader::GLES2Shader(ShaderType type, const ShaderDesc& desc, const char* srcBuffer, ui32 size)
		: Shader(type, desc, srcBuffer, size)
	{
		create("");
	}

	GLES2Shader::~GLES2Shader()
	{
		if (m_hShader)
		{
			OGLESDebug(glDeleteShader(m_hShader));
			m_hShader = 0;
		}
	}

	// 替换include
	void GLES2Shader::replaceInclude()
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

				// 递归替换
				replaceInclude();
			}
		}
	}

	void GLES2Shader::create(const String& filename)
	{
		if (!m_desc.macros.empty())
		{
			m_desc.macros = "\r\n" + m_desc.macros + "\r\n";

			size_t pos = m_srcData.find_first_of('\n') + 1;
			m_srcData.insert(pos, m_desc.macros);
		}

		// Windows平台，Debug模式，开启Nsight Shader调试
#if defined(ECHO_PLATFORM_WINDOWS) && defined(ECHO_DEBUG)
		size_t pos = m_srcData.find_first_of('\n') + 1;
		if (m_shaderType==ST_VERTEXSHADER)
			m_srcData.insert(pos, "#line 29\r\n");
		else if (m_shaderType==ST_PIXELSHADER)
			m_srcData.insert(pos, "#line 30\r\n");
#endif

		switch (m_shaderType)
		{
			case ST_VERTEXSHADER:
			{
				m_hShader = OGLESDebug(glCreateShader(GL_VERTEX_SHADER));
				if (!m_hShader)
				{
					EchoLogError("Create vertex Shader [%s] failed.", filename.c_str());
				}
			}
			break;
			case ST_PIXELSHADER:
			{
				m_hShader = OGLESDebug(glCreateShader(GL_FRAGMENT_SHADER));
				if (!m_hShader)
				{
					EchoLogError("Create pixel Shader [%s] failed.", filename.c_str());
				}
			} break;
			default:
			{
				EchoLogError("Unknown shader type, create Shader [%s] failed.", filename.c_str());
			}break;
		}

		// bind shader source code to the shader object
		const GLchar* srcData = &m_srcData[0];
		OGLESDebug(glShaderSource(m_hShader, 1, &srcData, NULL));

		// compile shader.
		OGLESDebug(glCompileShader(m_hShader));

		// check to see if the vertex shader compiled successfully.
		GLint status;
		OGLESDebug(glGetShaderiv(m_hShader, GL_COMPILE_STATUS, &status));
		if (status != 1)
		{
			m_validata = false;
			// get the size of the string containing the information log for the failed shader compilation message.
			GLint logSize = 0;
			OGLESDebug(glGetShaderiv(m_hShader, GL_INFO_LOG_LENGTH, &logSize));

			// increment the size by one to handle also the null terminator.
			++logSize;

			// create a char buffer to hold the info log.
			char* szLogInfo = (char*)EchoMalloc(logSize);

			// retrieve the info log.
			GLint realsize;
			OGLESDebug(glGetShaderInfoLog(m_hShader, logSize, &realsize, szLogInfo));

			String errMsg = szLogInfo;
			EchoSafeFree(szLogInfo);

			EchoLogError("Compile Shader [%s] Error: \n%s", filename.c_str(), errMsg.c_str());
		}

		// 移除数据
		m_srcData.clear();
		m_srcData.shrink_to_fit();
	}

	void GLES2Shader::setProgramHandle(ui32 hProgram)
	{
		m_hProgram = hProgram;
	}

	GLuint GLES2Shader::getShaderHandle() const
	{
		return m_hShader;
	}
}
