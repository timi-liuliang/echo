#pragma once

#include "engine/core/resource/Res.h"
#include "engine/core/render/base/Texture.h"
#include "TextureAtlas.h"

namespace Echo
{
    class TextureAtla : public Res
    {
        ECHO_RES(TextureAtla, Res, ".atla", Res::create<TextureAtla>, TextureAtla::load);

    public:
		TextureAtla();
		TextureAtla(const ResourcePath& path);
        virtual ~TextureAtla();

        // load | save
        static Res* load(const ResourcePath& path);
        virtual void save() override;

    protected:
        String          m_atlaName;
        TextureAtlasPtr m_owner;
    };
    typedef ResRef<TextureAtla> TextureAtlaPtr;
}
