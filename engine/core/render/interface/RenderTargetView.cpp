#include "RenderTargetView.h"
#include "FrameBuffer.h"

namespace Echo
{
	RenderTargetView::RenderTargetView(PixelFormat pixFmt, ui32 width, ui32 height)
		: RenderView(RVT_RENDERTARGET, pixFmt)
		, m_attIdx(-1)
		, m_width(width)
		, m_height(height)
	{
	}

	RenderTargetView::~RenderTargetView()
	{
	}

	int RenderTargetView::getAttachedIndex() const
	{
		return m_attIdx;
	}

	ui32 RenderTargetView::getWidth() const
	{
		return m_width;
	}

	ui32 RenderTargetView::getHeight() const
	{
		return m_height;
	}
}