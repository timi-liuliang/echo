#include "light_module.h"
#include "light.h"

namespace Echo
{
	LightModule::LightModule()
	{

	}

	void LightModule::registerTypes()
	{
		Class::registerType<Light>();
	}
}