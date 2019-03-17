#include "font_library.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    FontLibrary::FontLibrary()
    {
        FT_Error result = FT_Init_FreeType(&m_library);
        if(result==FT_Err_Ok)
        {
            
        }
        else
        {
            EchoLogError("UiModule FreeType init failed.");
        }
    }
    
    FontLibrary::~FontLibrary()
    {
        
    }
    
    FontLibrary* FontLibrary::instance()
    {
        static FontLibrary* inst = EchoNew(FontLibrary);
        return inst;
    }
    
    FontGlyph FontLibrary::getFontGlyph()
    {
        return FontGlyph();
    }
    
    bool FontLibrary::loadFace(const char* filePath)
    {
        return true;
    }
    
    bool FontLibrary::unloadFace(const char* filePath)
    {
        return true;
    }
}
