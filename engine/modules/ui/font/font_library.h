#pragma once

#include "font_glyph.h"
#include "font_face.h"
#include "font_texture.h"

namespace Echo
{
    class FontLibrary
    {
    public:
        // instance
        static FontLibrary* instance();
        
        // get glyph
        FontGlyph* getFontGlyph(i32 id);
        
    public:
        // face manager
        bool loadFace(const char* filePath);
        bool unloadFace(const char* filePath);
        
    private:
        FontLibrary();
        ~FontLibrary();
        
    private:
        FT_Library					m_library;
		map<i32, FontGlyph*>::type	m_glyphs;
		vector<FontFace*>::type		m_fontFaces;
		vector<FontTexture*>::type	m_fontTextures;
    };
}
