#include "render_view.h"
#include "renderer.h"

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