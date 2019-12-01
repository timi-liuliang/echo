#include "mt_texture.h"
#include "engine/core/io/IO.h"
#include "base/image/Image.h"

namespace Echo
{
    MTTexture2D::MTTexture2D(const String& pathName)
        : Texture(pathName)
    {
        
    }

    MTTexture2D::~MTTexture2D()
    {
        
    }

    bool MTTexture2D::load()
    {
        MemoryReader memReader(getPath());
        if (memReader.getSize())
        {
            Buffer commonTextureBuffer(memReader.getSize(), memReader.getData<ui8*>(), false);
            Image* image = Image::CreateFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
            if (image)
            {
                m_isCompressed = false;
                m_compressType = Texture::CompressType_Unknown;
                m_width = image->getWidth();
                m_height = image->getHeight();
                m_depth = image->getDepth();
                m_pixFmt = image->getPixelFormat();
                m_numMipmaps = image->getNumMipmaps() ? image->getNumMipmaps() : 1;
                ui32 pixelsSize = PixelUtil::CalcSurfaceSize(m_width, m_height, m_depth, m_numMipmaps, m_pixFmt);
                Buffer buff(pixelsSize, image->getData(), false);

                setSurfaceData( 0, m_pixFmt, m_usage, m_width, m_height, buff);
                
                EchoSafeDelete(image, Image);

                return true;
            }
        }

        return false;
    }

    void MTTexture2D::setSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff)
    {

    }
}
