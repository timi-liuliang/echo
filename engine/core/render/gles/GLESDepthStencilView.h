#pragma once

#include "interface/DepthStencilView.h"

namespace Echo
{
	class GLES2DepthStencilView : public DepthStencilView
	{
	public:
		GLES2DepthStencilView(PixelFormat pixFmt, ui32 width, ui32 height);
		~GLES2DepthStencilView();

	public:
		void		onAttached();
		void		onDetached();

		GLuint		getRBOHandle() const;

	private:
		GLuint		m_hRBO;
	};
}
