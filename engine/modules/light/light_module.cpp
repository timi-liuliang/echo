#include "light_module.h"
#include "light_settings.h"

namespace Echo
{
	LightModule::LightModule()
	{

	}

	void LightModule::registerTypes()
	{
		Class::registerType<LightSettings>();
	}
}