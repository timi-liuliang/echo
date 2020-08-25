#pragma once

#include "particle.h"

namespace Echo
{
    class ParticleGroup
    {
    public:
        ParticleGroup();
        ~ParticleGroup();
        
        // tick
        void tick(float elapsedTime);
        
    private:
        vector<Particle>::type  m_particles;
    };
}

