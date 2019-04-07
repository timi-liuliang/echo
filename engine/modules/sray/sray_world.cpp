#include "sray_world.h"
#include "engine/core/log/Log.h"
#include "engine/core/scene/node_tree.h"

namespace Echo
{
    SRayDevice::SRayDevice()
    {
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
