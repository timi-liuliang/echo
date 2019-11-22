#include "Renderer.h"
#include "base/FrameBuffer.h"
#include "base/ShaderProgram.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
    FrameBuffer::FrameBuffer()
    {
        m_views.assign(nullptr);
    }

	FrameBuffer::FrameBuffer( ui32 id, ui32 width, ui32 height)
		: m_id( id)
        , m_width(width)
        , m_height(height)
	{
        m_views.assign(nullptr);
	}

	FrameBuffer::~FrameBuffer()
	{
	}

	void FrameBuffer::bindMethods()
	{

	}
}
