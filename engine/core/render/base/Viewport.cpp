#include "Viewport.h"

namespace Echo
{
	Viewport::Viewport(int _left, int _top, ui32 _width, ui32 _height)
		: m_left(_left)
		, m_top(_top)
		, m_width(_width)
		, m_height(_height)
	{
	}

	Viewport::~Viewport()
	{
	}

	int Viewport::getLeft() const
	{
		return m_left;
	}

	int Viewport::getTop() const
	{
		return m_top;
	}

	ui32 Viewport::getWidth() const
	{
		return m_width;
	}

	ui32 Viewport::getHeight() const
	{
		return m_height;
	}

	void Viewport::resize(int left, int top, ui32 width, ui32 height)
	{
		m_left = left;
		m_top = top;
		m_width = width;
		m_height = height;
	}
}
