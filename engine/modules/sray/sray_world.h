#pragma once

#include "engine/core/scene/node.h"
#include <radeon_rays.h>
#include "sray_camera.h"

namespace Echo
{
    class SRayWorld : public Object
    {
        ECHO_SINGLETON_CLASS(SRayWorld, Object)
        
    public:
        SRayWorld();
        virtual ~SRayWorld();
        
        // instance
        static SRayWorld* instance();
        
        // step
        void step(float elapsedTime);
        
    private:
        ui32                            m_deviceCount;
        RadeonRays::DeviceInfo          m_deviceInfo;
        RadeonRays::IntersectionApi*    m_intersectionApi = nullptr;
		SRayCamera						m_camera;
    };
}
