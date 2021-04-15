#pragma once

#include "font_texture.h"

namespace Echo
{
    struct FontGlyph
    {
		FontTexture*	m_texture = nullptr;
		i32				m_nodeIndex = 0;

		FontGlyph();
		~FontGlyph();

		// width|height in pixels
		float getWidth();
		float getHeight();

		// get uv
		Vector4 getUV() const { return m_texture->getViewport(m_nodeIndex); }
    };
}
