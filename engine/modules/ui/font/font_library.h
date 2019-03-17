#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

namespace Echo
{
    class FontLibrary
    {
    public:
        FontLibrary();
        ~FontLibrary();
        
        // get glyph
        
    public:
        // face manager
        bool loadFace(const char* filePath);
        bool unloadFace(const char* filePath);
        
    private:
        FT_Library m_library;
    };
}
