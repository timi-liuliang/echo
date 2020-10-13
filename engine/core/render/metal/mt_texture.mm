#include "mt_texture.h"
#include "mt_mapping.h"
#include "mt_renderer.h"
#include "mt_render_state.h"
#include "engine/core/io/IO.h"
#include "base/image/image.h"

namespace Echo
{
    MTTexture2D::MTTexture2D(const String& pathName)
        : Texture(pathName)
    {
        
    }

    MTTexture2D::~MTTexture2D()
    {
        
    }

    id<MTLSamplerState> MTTexture2D::getMTSamplerState()
    {
        const MTSamplerState* mtSamplerState = ECHO_DOWN_CAST<const MTSamplerState*>(getSamplerState());
        return mtSamplerState ? mtSamplerState->getMTSamplerState() : id<MTLSamplerState>();
    }

    void MTTexture2D::convertFormat(Image* image)
    {
        PixelFormat format = image->getPixelFormat();
        if(format == PixelFormat::PF_RGB8_SNORM)        image->convertFormat( PixelFormat::PF_RGBA8_SNORM);
        else if(format == PixelFormat::PF_RGB8_UNORM)   image->convertFormat( PixelFormat::PF_RGBA8_UNORM);
        else if(format == PixelFormat::PF_RGB8_UINT)    image->convertFormat( PixelFormat::PF_RGBA8_UINT);
        else if(format == PixelFormat::PF_RGB8_SINT)    image->convertFormat( PixelFormat::PF_RGBA8_SINT);
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
                // metal doesn't support rgb format
                convertFormat(image);
                
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
        reset();
        
        if(!m_mtTextureDescriptor)
        {
            m_mtTextureDescriptor = [[MTLTextureDescriptor alloc] init];
            
            // Indicate that each pixel has a blue, green, red, and alpha channel, where each channel is
            // an 8-bit unsigned normalized value (i.e. 0 maps to 0.0 and 255 maps to 1.0)
            m_mtTextureDescriptor.pixelFormat = MTMapping::MapPixelFormat(pixFmt);

             // Set the pixel dimensions of the texture
            m_mtTextureDescriptor.width = width;
            m_mtTextureDescriptor.height = height;
            
            // usage
            if(PixelUtil::IsDepth(pixFmt))
            {
                m_mtTextureDescriptor.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
                m_mtTextureDescriptor.storageMode = MTLStorageModePrivate;
            }
            
            // Create the texture from the device by using the descriptor
            id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
            if(device)
                m_mtTexture = [device newTextureWithDescriptor:m_mtTextureDescriptor];
            
            if(m_mtTexture && buff.getData())
            {
                MTLRegion region = { { 0, 0, 0 }, { width, height, 1}};
                i32 bytesPerRow = PixelUtil::GetPixelSize(pixFmt) * width;
                
                [m_mtTexture replaceRegion:region mipmapLevel:0 withBytes:buff.getData() bytesPerRow:bytesPerRow];
            }
        }
    }

    bool MTTexture2D::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
    {
        Buffer buff(size, data, false);
        setSurfaceData(0, format, usage, width, height, buff);
        
        return true;
    }

    void MTTexture2D::reset()
    {
        if(m_mtTexture)
        {
            [m_mtTexture release];
            m_mtTexture = nullptr;
        }

        if(m_mtTextureDescriptor)
        {
            [m_mtTextureDescriptor release];
            m_mtTextureDescriptor = nullptr;
        }
    }
}
