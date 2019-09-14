#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/IO.h"
#include "font_glyph.h"
#include "font_texture.h"

namespace Echo
{
    class FontFace
    {
    public:
        FontFace(FT_Library& library, const char* filePath);
        ~FontFace();
        
        // file
        const String& getFile() const { return m_file;}
        
        // get glyph
        FontGlyph* getGlyph(i32 charCode, i32 fontSize);
        
    private:
        // load glyph
        FontGlyph* loadGlyph(i32 charCode, i32 fontSize);
        
        // copy glyph bitmap to texture
        FontGlyph* copyGlyphToTexture(i32 charCode, FT_GlyphSlot glyphSlot);
        bool copyGlyphToBitmap(Color* oColor, i32& ioWidth, i32& ioHeight, i32 charCode, FT_GlyphSlot glyphSlot);

		// new glyph
		FontGlyph* newGlyph(i32 charCode, FontTexture* texture, i32 nodeIndex);
        
    private:
        String						m_file;
		MemoryReader*				m_memory = nullptr;
        FT_Face						m_face;
		map<i32, FontGlyph*>::type	m_glyphs;
        vector<FontTexture*>::type  m_fontTextures;
    };
}
