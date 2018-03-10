#pragma once

#include "RenderView.h"

namespace Echo
{
	/**
	 * Éî¶ÈÄ£°å
	 */
	class ECHO_EXPORT_RENDER DepthStencilView : public RenderView
	{
	public:
		DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height);
		virtual ~DepthStencilView();

	public:
		virtual void		onAttached() = 0;
		virtual void		onDetached() = 0;

		virtual ui32		getWidth() const;
		virtual ui32		getHeight() const;

	protected:
		ui32				m_width;
		ui32				m_height;
	};
}
