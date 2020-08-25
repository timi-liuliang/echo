#pragma once

#include "engine/core/resource/Res.h"

namespace Echo
{
    class Particle
    {
    public:
        Particle();
        ~Particle();
        
    private:
        Vector3 m_position;
        Vector3 m_velocity;
        Color   m_color;
        float   m_life;
    };
}

