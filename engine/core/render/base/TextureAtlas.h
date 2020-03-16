#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
    class TextureAtlas : public Res
    {
        ECHO_RES(TextureAtlas, Res, ".atlas", Res::create<TextureAtlas>, TextureAtlas::load);

    public:
        TextureAtlas();
        TextureAtlas(const ResourcePath& path);
        virtual ~TextureAtlas();

        // load
        static Res* load(const ResourcePath& path);

        // save
        virtual void save() override;

    protected:

    };
    typedef ResRef<TextureAtlas> TextureAtlasPtr;
}
