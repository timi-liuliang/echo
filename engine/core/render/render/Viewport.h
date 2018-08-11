#pragma once

#include <engine/core/math/Matrix4.h>
#include "RenderDef.h"

namespace Echo
{
	struct Viewport
	{	
		int			m_left;
		int			m_top;
		ui32		m_width;
		ui32		m_height;

		Viewport() {}
		Viewport(int left, int top, ui32 width, ui32 height);
		virtual ~Viewport();

		int			getLeft() const;
		int			getTop() const;
		int			getRight() const { return m_left + m_width; }
		int			getBottom() const { return m_top + m_height; }
		ui32		getWidth() const;
		ui32		getHeight() const;

		void		resize(int left, int top, ui32 width, ui32 height);
	};
}
