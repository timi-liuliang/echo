#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H
#include "engine/core/util/StringUtil.h"

namespace Echo
{
    class FontFace
    {
    public:
        FontFace(FT_Library& library, const char* filePath);
        ~FontFace();
        
    private:
        String      m_file;
        FT_Face     m_face;
    };
}
