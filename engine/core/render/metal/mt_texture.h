#pragma once

#include "base/image/image.h"
#include "engine/core/render/base/texture.h"
#include "mt_render_base.h"

namespace Echo
{
	class MTTexture2D: public Texture
	{
    public:
        MTTexture2D(const String& pathName);
        virtual ~MTTexture2D();
        
        // type
        virtual TexType getType() const override { return TT_2D; }
        
        // get mtl texture
        id<MTLTexture> getMTTexture() { return m_mtTexture; }
        
        // get sampler state
        id<MTLSamplerState> getMTSamplerState();
        
        // load
        virtual bool load() override;
        
        // reset
        void reset();
        
    public:
        // update
        virtual bool updateTexture2D(PixelFormat format, TexUsage usage, i32 width, i32 height, void* data, ui32 size);
        
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
