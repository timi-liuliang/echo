#include "font_library.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    FontLibrary::FontLibrary()
    {
        FT_Error result = FT_Init_FreeType(&m_library);
        if(result!=FT_Err_Ok)
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
    
    FontGlyph* FontLibrary::getFontGlyph(i32 charCode, const ResourcePath& fontPath, i32 fontSize)
    {
        FontFace* fontFace = loadFace( fontPath.getPath().c_str());
        if(fontFace)
        {
            return fontFace->getGlyph(charCode, fontSize);
        }
        
        return nullptr;
    }
    
	FontFace* FontLibrary::loadFace(const char* filePath)
    {
        // if exist, return it
        for(FontFace* fontFace : m_fontFaces)
        {
            if(fontFace->getFile()==filePath)
                return fontFace;
        }
        
        // create new
		FontFace* face = EchoNew(FontFace(m_library, filePath));
		m_fontFaces.push_back(face);

        return face;
    }
    
    bool FontLibrary::unloadFace(const char* filePath)
    {
        return true;
    }
}
