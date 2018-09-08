#include "RenderView.h"
#include "Renderer.h"

namespace Echo
{
	RenderView::RenderView(RenderViewType type, PixelFormat pixFmt)
		: m_type(type)
		, m_pixFmt(pixFmt)
	{
	}

	RenderView::~RenderView()
	{
	}
}