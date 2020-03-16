#include "sray_world.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
    SRayWorld::SRayWorld()
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
    
    SRayWorld::~SRayWorld()
    {
    }
    
    void SRayWorld::bindMethods()
    {
    }
    
    SRayWorld* SRayWorld::instance()
    {
        static SRayWorld* inst = EchoNew(SRayWorld);
        return inst;
    }
    
    void SRayWorld::step(float elapsedTime)
    {
		m_camera.update();
    }
}
