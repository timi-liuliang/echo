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

	SRayModule* SRayModule::instance()
	{
		static SRayModule* inst = EchoNew(SRayModule);
		return inst;
	}

	void SRayModule::bindMethods()
	{

	}
    
	void SRayModule::registerTypes()
	{
        Class::registerType<SRayDevice>();
	}
}
