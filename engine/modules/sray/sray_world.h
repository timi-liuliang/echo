#pragma once

#include "engine/core/scene/node.h"
#include <radeon_rays.h>

namespace Echo
{
    class SRayDevice : public Object
    {
        ECHO_SINGLETON_CLASS(SRayDevice, Object)
        
    public:
        SRayDevice();
        virtual ~SRayDevice();
        
        // instance
        static SRayDevice* instance();
        
        // step
        void step(float elapsedTime);
        
    private:
        RadeonRays::IntersectionApi*    m_intersectionApi = nullptr;
    };
}
