#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/DataStream.h"
#include "font_glyph.h"

namespace Echo
{
    class FontFace
    {
    public:
        FontFace(FT_Library& library, const char* filePath);
        ~FontFace();
        
    private:
        String						m_file;
		MemoryReader*				m_memory = nullptr;
        FT_Face						m_face;
		map<i32, FontGlyph*>::type	m_glyphs;
    };
}
