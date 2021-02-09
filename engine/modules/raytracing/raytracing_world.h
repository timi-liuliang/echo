#pragma once

#include "engine/core/scene/node.h"
#include <radeon_rays.h>
#include "raytracing_camera.h"

namespace Echo
{
    class RaytracingWorld : public Object
    {
        ECHO_SINGLETON_CLASS(RaytracingWorld, Object)
        
    public:
        RaytracingWorld();
        virtual ~RaytracingWorld();
        
        // instance
        static RaytracingWorld* instance();
        
        // step
        void step(float elapsedTime);

        // render
        void render();

	public:
		// get intersection API
		RadeonRays::IntersectionApi* getIntersectionApi() { return m_intersectionApi; }
        
    private:
        ui32                            m_deviceCount;
        RadeonRays::DeviceInfo          m_deviceInfo;
        RadeonRays::IntersectionApi*    m_intersectionApi = nullptr;
		RaytracingCamera				m_camera;
    };
}
