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
        FontGlyph* getFontGlyph(i32 charCode, const ResourcePath& fontPath, i32 fontSize);
        
    public:
        // face manager
		FontFace* loadFace(const char* filePath);
        bool unloadFace(const char* filePath);
        
    private:
        FontLibrary();
        ~FontLibrary();
        
    private:
        FT_Library					m_library;
		vector<FontFace*>::type		m_fontFaces;
    };
}
