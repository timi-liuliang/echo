#pragma once

#include "RenderView.h"

namespace Echo
{
	class RenderTargetView : public RenderView
	{
	public:
		RenderTargetView(PixelFormat pixFmt, ui32 width, ui32 height);
		virtual ~RenderTargetView();

	public:
		virtual void		onAttached(ui32 idx) = 0;
		virtual void		onDetached() = 0;

		virtual int			getAttachedIndex() const;
		virtual ui32		getWidth() const;
		virtual ui32		getHeight() const;

	protected:
		int					m_attIdx;
		ui32				m_width;
		ui32				m_height;
	};
}