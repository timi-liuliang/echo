#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
    class ParticleSystem : public Res
    {
        ECHO_RES(ParticleSystem, Res, ".particlesystem", Res::create<ParticleSystem>, ParticleSystem::load);

    public:
        ParticleSystem();
        ~ParticleSystem();

        // load|save
        static Res* load(const ResourcePath& path);
        virtual void save() override;

    protected:
    };
    typedef Echo::ResRef<Echo::ParticleSystem> ParticleSystemPtr;
}

