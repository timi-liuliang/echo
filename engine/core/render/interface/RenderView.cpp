#include "RenderView.h"
#include "Renderer.h"

namespace Echo
{
	RenderView::RenderView(ui32 width, ui32 height, PixelFormat pixelFormat)
        : m_width(width)
        , m_height(height)
        , m_format(pixelFormat)
	{
	}

	RenderView::~RenderView()
	{
	}
}