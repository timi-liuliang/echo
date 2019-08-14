#include "Renderer.h"
#include "interface/FrameBuffer.h"
#include "interface/ShaderProgram.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
	FrameBuffer::FrameBuffer( ui32 id, ui32 width, ui32 height, PixelFormat format, const Options& option )
		: m_isHasMSAA(option.msaa)
        , m_id( id)
		, m_pixelFormat( format)
        , m_isHasDepth(option.depth)
		, m_width( width)
		, m_height( height)
	{
	}

	FrameBuffer::~FrameBuffer()
	{
		m_bindTexture = NULL;
		m_depthTexture = NULL;
	}

	void FrameBuffer::bindMethods()
	{

	}

    Echo::ui32 FrameBuffer::getMemorySize()
    {
        ui32 memorySize = 0;
        if (m_bindTexture)
            memorySize += PixelUtil::CalcLevelSize(m_bindTexture->getWidth(), m_bindTexture->getHeight(), m_bindTexture->getDepth(), 0, m_bindTexture->getPixelFormat());

        if (m_isHasDepth)
            memorySize += PixelUtil::CalcLevelSize(m_depthTexture->getWidth(), m_depthTexture->getHeight(), m_depthTexture->getDepth(), 0, m_depthTexture->getPixelFormat());

        return memorySize;
    }
}
