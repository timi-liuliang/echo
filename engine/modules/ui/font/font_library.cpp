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
    
    FontGlyph* FontLibrary::getFontGlyph(i32 id)
    {
		if (m_fontTextures.empty())
		{
			// create new texture
			FontTexture* newTexture = EchoNew(FontTexture(512, 512));
			newTexture->refreshTexture();
			m_fontTextures.push_back(newTexture);
		}

		if (m_glyphs.empty())
		{
			FontGlyph* fontGlyph = new FontGlyph;
			fontGlyph->m_texture = m_fontTextures[0]->getTexture();
			m_glyphs[id] = fontGlyph;
		}

		return m_glyphs[id];
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
