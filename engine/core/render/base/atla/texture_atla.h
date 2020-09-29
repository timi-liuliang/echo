#pragma once

#include "engine/core/resource/Res.h"
#include "engine/core/render/base/texture.h"
#include "texture_atlas.h"

namespace Echo
{
    class TextureAtla : public Res
    {
        ECHO_RES(TextureAtla, Res, ".atla", Res::create<TextureAtla>, TextureAtla::load);

    public:
		TextureAtla();
		TextureAtla(const ResourcePath& path);
        virtual ~TextureAtla();

        // get texture
        TexturePtr getTexture() { return m_owner ? m_owner->getTexture() : nullptr; }

        // view port
        Vector4 getViewport();
        Vector4 getViewportNormalized();

        // load | save
        static Res* load(const ResourcePath& path);
        virtual void save() override;

    protected:
        String          m_atlaName;
        TextureAtlasPtr m_owner;
    };
    typedef ResRef<TextureAtla> TextureAtlaPtr;
}
