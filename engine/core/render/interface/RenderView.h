#pragma once

#include "PixelFormat.h"

namespace Echo
{
	class RenderView
	{
	public:
		enum RenderViewType
		{
			RVT_RENDERTARGET, 
			RVT_DEPTHSTENCIL, 
		};

	public:
		RenderView(RenderViewType type, PixelFormat pixFmt);
		virtual ~RenderView();

		// get view type
		virtual RenderViewType getRenderViewType() const { return m_type; }

		// get pixel format
		virtual PixelFormat	getPixelFormat() const { return m_pixFmt; }

	protected:
		RenderViewType		m_type;
		PixelFormat			m_pixFmt;
	};
}
