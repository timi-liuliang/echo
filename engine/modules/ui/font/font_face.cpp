#include "font_face.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    FontFace::FontFace(FT_Library& library, const char* filePath)
        : m_file(filePath)
    {
		m_memory = EchoNew(MemoryReader(filePath));
		if (m_memory->getSize())
		{
			FT_Error error = FT_New_Memory_Face(library, m_memory->getData<Byte*>(), m_memory->getSize(), 0, &m_face);
			if (error == FT_Err_Unknown_File_Format)
			{
				EchoLogError("the font file [%s] could be opened and read, but it appears that its font format is unsupported", filePath);
			}
			else if (error)
			{
				EchoLogError("font file [%s] could not be opened or read, or that it is broken...", filePath);
			}
		}
    }
    
    FontFace::~FontFace()
    {
        
    }
    
    // get glyph
    FontGlyph* FontFace::getGlyph(i32 charCode, i32 fontSize)
    {
        // if exist, return it
        auto it = m_glyphs.find(charCode);
        if(it!=m_glyphs.end())
        {
            return it->second;
        }
        
        // create new one
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
}
