#include "sray_module.h"
#include "sray_world.h"

namespace Echo
{
	SRayModule::SRayModule()
	{
	}

    SRayModule::~SRayModule()
    {
        EchoSafeDeleteInstance(SRayDevice);
    }
    
	void SRayModule::registerTypes()
	{
        Class::registerType<SRayDevice>();
	}
}
