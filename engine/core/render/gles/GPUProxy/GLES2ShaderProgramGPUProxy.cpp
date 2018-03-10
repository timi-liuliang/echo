#include "GLES2RenderStd.h"
#include "engine/core/Util/Exception.h"
#include "engine/core/Util/Array.hpp"
#include "Render/ShaderProgram.h"
#include "GLES2Mapping.h"
#include "GLES2ShaderProgramGPUProxy.h"

namespace Echo
{

	GLES2ShaderProgramGPUProxy::GLES2ShaderProgramGPUProxy()
		: m_hProgram(0)
		, m_link_status(0)
	{

	}

	GLES2ShaderProgramGPUProxy::~GLES2ShaderProgramGPUProxy()
	{
	}

	void GLES2ShaderProgramGPUProxy::linkShaders()
	{
// 		for (ui32 i = 0; i < RenderInput::VS_MAX; ++i)
// 		{
// 			String strName = GLES2Mapping::MapVertexSemanticString((RenderInput::VertexSemantic)i);
// 			OGLESDebug(glBindAttribLocation(m_hProgram, i, strName.c_str()));
// 		}

		OGLESDebug(glLinkProgram(m_hProgram));

		// Check the status of the link.
		OGLESDebug(glGetProgramiv(m_hProgram, GL_LINK_STATUS, &m_link_status));
		if (m_link_status != 1)
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
		}
	}

	void GLES2ShaderProgramGPUProxy::uniformsReflection(ShaderProgram::UniformArray* uniform_array)
	{
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

			ShaderProgram::Uniform desc;
			desc.m_name = StringUtil::Replace(unifromName, "[0]", "").c_str();
			desc.m_type = GLES2Mapping::MapUniformType(uniformType);
			desc.m_count = uniformSize;
			desc.m_sizeInBytes = desc.m_count * ShaderProgram::getUniformByteSizeByUniformType(desc.m_type);
			desc.m_location = glGetUniformLocation(m_hProgram, desc.m_name.c_str());
			(*uniform_array)[desc.m_location] = desc;
		}
	}

	void GLES2ShaderProgramGPUProxy::attribReflection(GLES2ShaderProgram::AttribLocationArray* attrib_array)
	{
		for (ui32 i = 0; i < RenderInput::VS_MAX; ++i)
		{
			String strName = GLES2Mapping::MapVertexSemanticString((RenderInput::VertexSemantic)i);
			GLint loc = OGLESDebug(glGetAttribLocation(m_hProgram, strName.c_str()));
			if (loc != -1)
			{
				(*attrib_array)[i] = loc;
			}
		}
	}

	void GLES2ShaderProgramGPUProxy::bindUniforms(ShaderParamType type, int location, int count, Byte* value)
	{
		switch (type)
		{
		case SPT_VEC4:		OGLESDebug(glUniform4fv(location, count, (GLfloat*)value));					break;
		case SPT_MAT4:		OGLESDebug(glUniformMatrix4fv(location, count, false, (GLfloat*)value));	break;
		case SPT_INT:		OGLESDebug(glUniform1iv(location, count, (GLint*)value));					break;
		case SPT_FLOAT:		OGLESDebug(glUniform1fv(location, count, (GLfloat*)value));					break;
		case SPT_VEC2:		OGLESDebug(glUniform2fv(location, count, (GLfloat*)value));					break;
		case SPT_VEC3:		OGLESDebug(glUniform3fv(location, count, (GLfloat*)value));					break;
		case SPT_TEXTURE:	OGLESDebug(glUniform1i(location, *(ui32*)value));							break;
		default:			EchoAssertX(0, "unknow shader param format!");								break;
		}
	}

	void GLES2ShaderProgramGPUProxy::createProgram()
	{
		m_hProgram = OGLESDebug(glCreateProgram());
		if (m_hProgram == 0)
		{
			EchoException("Create GLES2ShaderProgram failed.");
		}
	}

	void GLES2ShaderProgramGPUProxy::deleteProgram()
	{
		OGLESDebug(glDeleteProgram(m_hProgram));
		m_hProgram = 0;
	}

	void GLES2ShaderProgramGPUProxy::attachShader(ui32 shader)
	{
		OGLESDebug(glAttachShader(m_hProgram, shader));
	}

	void GLES2ShaderProgramGPUProxy::detachShader(ui32 shader)
	{
		OGLESDebug(glDetachShader(m_hProgram, shader));
	}

	void GLES2ShaderProgramGPUProxy::useProgram()
	{
		OGLESDebug(glUseProgram(m_hProgram));
	}

}