#include "mt_texture.h"
#include "mt_mapping.h"
#include "mt_renderer.h"
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
            Image* image = Image::createFromMemory(commonTextureBuffer, Image::GetImageFormat(getPath()));
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
		return;

        if(!m_mtTextureDescriptor)
        {
            m_mtTextureDescriptor = [[MTLTextureDescriptor alloc] init];
            
            // Indicate that each pixel has a blue, green, red, and alpha channel, where each channel is
            // an 8-bit unsigned normalized value (i.e. 0 maps to 0.0 and 255 maps to 1.0)
            m_mtTextureDescriptor.pixelFormat = MTMapping::MapPixelFormat(pixFmt);

             // Set the pixel dimensions of the texture
            m_mtTextureDescriptor.width = width;
            m_mtTextureDescriptor.height = height;
            
            // Create the texture from the device by using the descriptor
            id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
            if(device)
                m_mtTexture = [device newTextureWithDescriptor:m_mtTextureDescriptor];
            
            if(m_mtTexture)
            {
                MTLRegion region = { { 0, 0, 0 }, { width, height, 1}};
                i32 bytesPerRow = PixelUtil::GetPixelSize(pixFmt) * width;
                
                [m_mtTexture replaceRegion:region mipmapLevel:0 withBytes:buff.getData() bytesPerRow:bytesPerRow];
            }
        }
    }
}
