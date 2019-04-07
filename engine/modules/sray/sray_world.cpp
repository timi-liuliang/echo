#include "sray_world.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
    SRayDevice::SRayDevice()
    {
        m_deviceCount = RadeonRays::IntersectionApi::GetDeviceCount();
        if(m_deviceCount>0)
        {
            // use first endable device
            RadeonRays::IntersectionApi::GetDeviceInfo( 0, m_deviceInfo);
            
            // set platform
            RadeonRays::IntersectionApi::SetPlatform( m_deviceInfo.platform);
            m_intersectionApi = RadeonRays::IntersectionApi::Create( 0);
        }
    }
    
    SRayDevice::~SRayDevice()
    {
    }
    
    void SRayDevice::bindMethods()
    {
    }
    
    SRayDevice* SRayDevice::instance()
    {
        static SRayDevice* inst = EchoNew(SRayDevice);
        return inst;
    }
    
    // step
    void SRayDevice::step(float elapsedTime)
    {
    }
}
