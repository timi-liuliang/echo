#pragma once

#include "font_glyph.h"
#include "font_face.h"

namespace Echo
{
    class FontLibrary
    {
    public:
        // instance
        static FontLibrary* instance();
        
        // get glyph
        FontGlyph getFontGlyph();
        
    public:
        // face manager
        bool loadFace(const char* filePath);
        bool unloadFace(const char* filePath);
        
    private:
        FontLibrary();
        ~FontLibrary();
        
    private:
        FT_Library m_library;
    };
}
