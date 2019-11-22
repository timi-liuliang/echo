#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
    class ImageSet : public Res
    {
        ECHO_RES(ImageSet, Res, ".imageset", Res::create<ImageSet>, ImageSet::load);

    public:
        ImageSet();
        ImageSet(const ResourcePath& path);
        virtual ~ImageSet();

        // load
        static Res* load(const ResourcePath& path);

        // save
        virtual void save() override;

    protected:

    };
    typedef ResRef<ImageSet> ImageSetPtr;
}
