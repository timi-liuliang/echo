#include "gles_render_base.h"
#include "gles_gpu_buffer.h"
#include "gles_mapping.h"
#include "gles_renderer.h"
#include <engine/core/util/Exception.h>
#include <engine/core/log/Log.h>

namespace Echo
{
	GLESGPUBuffer::GLESGPUBuffer(GPUBufferType type, Dword usage, const Buffer& buff)
		: GPUBuffer(type, usage, buff)
	{
		switch (type)
		{
			case GBT_VERTEX:	m_target = GL_ARRAY_BUFFER;				break;
			case GBT_INDEX:		m_target = GL_ELEMENT_ARRAY_BUFFER;		break;
			default:			EchoLogError("Unknown GPUBufferType."); break;
		}

		m_glUsage = GLESMapping::MapGPUBufferUsage(m_usage);

		// Generate an ID for the buffer.
		OGLESDebug(glGenBuffers(1, &m_hVBO));

		updateData(buff);
	}

	GLESGPUBuffer::~GLESGPUBuffer()
	{
		OGLESDebug(glDeleteBuffers(1, &m_hVBO));
	}

	bool GLESGPUBuffer::updateData(const Buffer& buff)
	{
		if (buff.getSize() > 0 || m_glUsage != GL_STATIC_DRAW)
		{
			m_size = buff.getSize();

			OGLESDebug(glBindBuffer(m_target, m_hVBO));
			//glBufferSubData(m_target, offset, buff.getSize(), buff.getData());
			OGLESDebug(glBufferData(m_target, buff.getSize(), buff.getData(), m_glUsage));

			return true;
		}

		EchoLogWarning("GLES2GPUBuffer::updateSubData failed");
		return false;
	}

	void GLESGPUBuffer::bindBuffer()
	{
		OGLESDebug(glBindBuffer(m_target, m_hVBO));
	}
}
