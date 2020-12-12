#pragma once

#include "base/image/image.h"
#include "engine/core/render/base/texture_render.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTTextureRender: public TextureRender
	{
    public:
        MTTextureRender(const String& pathName);
        virtual ~MTTextureRender();
        
        // get mtl texture
        id<MTLTexture> getMTTexture() { return m_mtTexture; }
        
        // get sampler state
        id<MTLSamplerState> getMTSamplerState();
        
        // reset
        void reset();
        
    public:
        // update
        virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size) override;
        
        // set surface data
        void setSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff);
        
    private:
        // convert format
        void convertFormat(Image* image);
        
    private:
        MTLTextureDescriptor*   m_mtTextureDescriptor = nullptr;
        id<MTLTexture>          m_mtTexture = nullptr;
    };
}
