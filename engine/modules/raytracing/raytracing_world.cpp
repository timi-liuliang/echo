#include "raytracing_world.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
    RaytracingWorld::RaytracingWorld()
    {
        try
        {
            m_deviceCount = RadeonRays::IntersectionApi::GetDeviceCount();
            if(m_deviceCount>0)
            {
                // set preference platform
                m_deviceInfo.platform = RadeonRays::DeviceInfo::Platform::kAny;
                
                // use first enable device
                RadeonRays::IntersectionApi::GetDeviceInfo( 0, m_deviceInfo);
                
                // set platform
                RadeonRays::IntersectionApi::SetPlatform( m_deviceInfo.platform);
                m_intersectionApi = RadeonRays::IntersectionApi::Create( 0);
            }
        }
        catch(...)
        {
            EchoLogError("RadeonRays load kernal failed");
        }
    }
    
    RaytracingWorld::~RaytracingWorld()
    {
    }
    
    void RaytracingWorld::bindMethods()
    {
    }
    
    RaytracingWorld* RaytracingWorld::instance()
    {
        static RaytracingWorld* inst = EchoNew(RaytracingWorld);
        return inst;
    }
    
    void RaytracingWorld::step(float elapsedTime)
    {
		m_camera.update();
    }
}
