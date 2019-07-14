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
        EchoSafeDelete(m_memory, MemoryReader);
        EchoSafeDeleteContainer(m_fontTextures, FontTexture);
        EchoSafeDeleteMap(m_glyphs, FontGlyph);
    }
    
    FontGlyph* FontFace::getGlyph(i32 charCode, i32 fontSize)
    {
        // if exist, return it
        auto it = m_glyphs.find(charCode);
        if(it!=m_glyphs.end())
        {
            return it->second;
        }
        
        // create new one
        return loadGlyph( charCode, fontSize);
    }
    
    FontGlyph* FontFace::loadGlyph(i32 charCode, i32 fontSize)
    {
        // get glyph index
        i32 glyphIndex = FT_Get_Char_Index( m_face, charCode);

		// set pixel size
		FT_Error error = FT_Set_Pixel_Sizes(m_face, fontSize * 2, fontSize * 2);
		if (error)
			return nullptr;
        
        // load glyph
        i32 loadFlags = FT_LOAD_DEFAULT;
        error = FT_Load_Glyph( m_face, glyphIndex, loadFlags);
        if(error)
            return nullptr;
        
        // convert to an anti-aliased bitmap
        error = FT_Render_Glyph(m_face->glyph, FT_RENDER_MODE_NORMAL);
        if(error)
            return nullptr;
        
        return copyGlyphToTexture(charCode, m_face->glyph);
    }
    
    FontGlyph* FontFace::copyGlyphToTexture(i32 charCode, FT_GlyphSlot glyphSlot)
    {
        // convert glyph to bitmap(color array)
        i32 glyphWidth = 128;
        i32 glyphHeight = 128;
        Color glyphBitmap[128*128];
        if(!copyGlyphToBitmap( glyphBitmap, glyphWidth, glyphHeight, charCode, glyphSlot))
            return nullptr;
        
        // try to insert to exist font texture
        for(FontTexture* fontTexture : m_fontTextures)
        {
			i32 nodeIndex = fontTexture->insert(glyphBitmap, glyphWidth, glyphHeight);
            if(nodeIndex !=-1)
            {
				fontTexture->refreshTexture();
				return newGlyph(charCode, fontTexture, nodeIndex);
            }
        }
        
        // create new one
        FontTexture* newTexture = EchoNew(FontTexture(512, 512));
        m_fontTextures.push_back(newTexture);
		i32 nodeIndex = newTexture->insert(glyphBitmap, glyphWidth, glyphHeight);
		if (nodeIndex != -1)
		{
			newTexture->refreshTexture();
			return newGlyph(charCode, newTexture, nodeIndex);
		}

        return nullptr;
    }
    
    bool FontFace::copyGlyphToBitmap(Color* oColor, i32& ioWidth, i32& ioHeight, i32 charCode, FT_GlyphSlot glyphSlot)
    {
        FT_Bitmap* bitmap = &glyphSlot->bitmap;
        if(ioWidth>=bitmap->width && ioHeight>=bitmap->rows)
        {
            for(i32 w=0; w<bitmap->width; w++)
            {
                for(i32 h=0; h<bitmap->rows; h++)
                {
                    i32 index = h * bitmap->width + w;
                    oColor[index].r = bitmap->buffer[index];
                    oColor[index].g = bitmap->buffer[index];
                    oColor[index].b = bitmap->buffer[index];
                    oColor[index].a = bitmap->buffer[index];
                }
            }
            
            ioWidth = bitmap->width;
            ioHeight = bitmap->rows;
            
            return true;
        }
        
        return false;
    }

	FontGlyph* FontFace::newGlyph(i32 charCode, FontTexture* texture, i32 nodeIndex)
	{
		// organize glyph data
		FontGlyph* fontGlyph = new FontGlyph;
		fontGlyph->m_texture = texture;
		fontGlyph->m_nodeIndex = nodeIndex;
		m_glyphs[charCode] = fontGlyph;

		return fontGlyph;
	}
}
