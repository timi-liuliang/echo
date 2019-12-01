#pragma once

#include "engine/core/render/base/Texture.h"
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
        
        // load
        virtual bool load() override;
        
    protected:
        // set surface data
        void setSurfaceData(int level, PixelFormat pixFmt, Dword usage, ui32 width, ui32 height, const Buffer& buff);
        
    private:
        MTLTextureDescriptor*   m_mtTextureDescriptor = nullptr;
        id<MTLTexture>          m_mtTexture;
    };
}
