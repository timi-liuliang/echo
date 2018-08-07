#include "render/DepthStencilView.h"

namespace Echo
{
	DepthStencilView::DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height)
		: RenderView(RVT_DEPTHSTENCIL, pixFmt)
		, m_width(width)
		, m_height(height)
	{
	}

	DepthStencilView::~DepthStencilView()
	{
	}

	ui32 DepthStencilView::getWidth() const
	{
		return m_width;
	}

	ui32 DepthStencilView::getHeight() const
	{
		return m_height;
	}
}
