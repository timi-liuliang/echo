#pragma once

namespace Echo
{
    struct FontGlyph
    {
		class Texture*	m_texture = nullptr;

		FontGlyph();
		~FontGlyph();
    };
}
