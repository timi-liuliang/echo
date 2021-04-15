#include "font_face.h"
#include "engine/core/log/Log.h"

#define DEFAULT_FONT_TEXTURE_SIZE	1024

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
        
        return copyGlyphToTexture(charCode, m_face->glyph, fontSize);
    }
    
    FontGlyph* FontFace::copyGlyphToTexture(i32 charCode, FT_GlyphSlot glyphSlot, i32 fontSize)
    {
        // convert glyph to bitmap(color array)
        i32 glyphWidth = 0;
        i32 glyphHeight = fontSize * 2;
        vector<Color>::type glyphBitmap;;
        if(!copyGlyphToBitmap( glyphBitmap, glyphWidth, glyphHeight, charCode, glyphSlot))
            return nullptr;
        
        // try to insert to exist font texture
        for(FontTexture* fontTexture : m_fontTextures)
        {
			i32 nodeIndex = fontTexture->insert(glyphBitmap.data(), glyphWidth, glyphHeight);
            if(nodeIndex !=-1)
            {
				fontTexture->refreshTexture();
				return newGlyph(charCode, fontTexture, nodeIndex);
            }
        }
        
        // create new one
        FontTexture* newTexture = EchoNew(FontTexture(DEFAULT_FONT_TEXTURE_SIZE, DEFAULT_FONT_TEXTURE_SIZE));
        m_fontTextures.emplace_back(newTexture);
		i32 nodeIndex = newTexture->insert(glyphBitmap.data(), glyphWidth, glyphHeight);
		if (nodeIndex != -1)
		{
			newTexture->refreshTexture();
			return newGlyph(charCode, newTexture, nodeIndex);
		}

        return nullptr;
    }
    
    bool FontFace::copyGlyphToBitmap(vector<Color>::type& oColor, i32& glyphWidth, i32& glyphHeight, i32 charCode, FT_GlyphSlot glyphSlot)
    {
        FT_Bitmap* bitmap = &glyphSlot->bitmap;
        glyphWidth = Math::Max<i32>(glyphWidth, bitmap->width * 1.3f);
        glyphHeight = Math::Max<i32>(glyphHeight, bitmap->rows);
        oColor.resize(glyphWidth * glyphHeight, Color(0.f, 0.f, 0.f, 0.f));

        if(glyphWidth>=bitmap->width && glyphHeight>=bitmap->rows)
        {
			i32 wOffset = (glyphWidth - bitmap->width) / 2;
			i32 hOffset = (glyphHeight - bitmap->rows) / 2;

            for(i32 w=0; w<bitmap->width; w++)
            {
                for(i32 h=0; h<bitmap->rows; h++)
                {
                    i32 index0 = h * bitmap->width + w;
					i32 index1 = (h + hOffset) * glyphWidth + w + wOffset;
                    oColor[index1].r = bitmap->buffer[index0];
                    oColor[index1].g = bitmap->buffer[index0];
                    oColor[index1].b = bitmap->buffer[index0];
                    oColor[index1].a = bitmap->buffer[index0];
                }
            }
            
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
