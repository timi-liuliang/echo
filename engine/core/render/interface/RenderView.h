#pragma once

#include "image/PixelFormat.h"

namespace Echo
{
	class RenderView
	{
	public:
		RenderView();
		virtual ~RenderView();

		// get pixel format
		virtual PixelFormat	getPixelFormat() const { return m_pixFmt; }

	protected:
        ui32				m_width;
        ui32				m_height;
		PixelFormat			m_pixFmt;
	};
}
