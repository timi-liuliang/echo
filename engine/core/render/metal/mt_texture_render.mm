#include "mt_texture_render.h"
#include "mt_mapping.h"
#include "mt_renderer.h"
#include "mt_render_state.h"
#include "engine/core/io/IO.h"
#include "base/image/image.h"

namespace Echo
{
    MTTextureRender::MTTextureRender(const String& pathName)
        : TextureRender(pathName)
    {
        
    }

    MTTextureRender::~MTTextureRender()
    {
        
    }

    id<MTLSamplerState> MTTextureRender::getMTSamplerState()
    {
        const MTSamplerState* mtSamplerState = ECHO_DOWN_CAST<const MTSamplerState*>(getSamplerState());
        return mtSamplerState ? mtSamplerState->getMTSamplerState() : id<MTLSamplerState>();
    }

    void MTTextureRender::convertFormat(Image* image)
    {
        PixelFormat format = image->getPixelFormat();
        if(format == PixelFormat::PF_RGB8_SNORM)        image->convertFormat( PixelFormat::PF_RGBA8_SNORM);
        else if(format == PixelFormat::PF_RGB8_UNORM)   image->convertFormat( PixelFormat::PF_RGBA8_UNORM);
        else if(format == PixelFormat::PF_RGB8_UINT)    image->convertFormat( PixelFormat::PF_RGBA8_UINT);
        else if(format == PixelFormat::PF_RGB8_SINT)    image->convertFormat( PixelFormat::PF_RGBA8_SINT);
    }

    void MTTextureRender::setSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff)
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

    bool MTTextureRender::updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size)
    {
        Buffer buff(size, data, false);
        setSurfaceData(0, format, usage, width, height, buff);
        
        return true;
    }

    void MTTextureRender::reset()
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
