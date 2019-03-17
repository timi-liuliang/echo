#include "font_face.h"
#include "engine/core/log/Log.h"

namespace Echo
{
    FontFace::FontFace(FT_Library& library, const char* filePath)
        : m_file(filePath)
    {
        FT_Error error = FT_New_Face( library, filePath, 0, &m_face);
        if (error == FT_Err_Unknown_File_Format )
        {
            EchoLogError("the font file [%s] could be opened and read, but it appears that its font format is unsupported", filePath);
        }
        else if ( error )
        {
            EchoLogError( "font file [%s] could not be opened or read, or that it is broken...", filePath);
        }
    }
    
    FontFace::~FontFace()
    {
        
    }
}
