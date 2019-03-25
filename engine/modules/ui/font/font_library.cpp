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
		if (m_fontTextures.empty())
		{
			// create new texture
			FontTexture* newTexture = EchoNew(FontTexture(512, 512));
			newTexture->refreshTexture();
			m_fontTextures.push_back(newTexture);
		}

		FontGlyph* fontGlyph = new FontGlyph;
		fontGlyph->m_texture = m_fontTextures[0]->getTexture();

		return fontGlyph;
    }
    
	FontFace* FontLibrary::loadFace(const char* filePath)
    {
		FontFace* face = EchoNew(FontFace(m_library, filePath));
		m_fontFaces.push_back(face);

        return face;
    }
    
    bool FontLibrary::unloadFace(const char* filePath)
    {
        return true;
    }

	// new glyph
	void FontLibrary::newGlyph(i32 charCode, const ResourcePath& fontPath, i32 fontSize)
	{
		FontFace* fontFace = loadFace(fontPath.getPath().c_str());
	}
}
