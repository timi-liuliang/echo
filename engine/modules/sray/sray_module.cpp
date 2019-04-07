#include "sray_module.h"
#include "sray_world.h"

namespace Echo
{
	SRayModule::SRayModule()
	{
	}

	void SRayModule::registerTypes()
	{
        Class::registerType<SRayDevice>();
	}
}
