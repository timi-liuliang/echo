#include "light_module.h"
#include "light.h"
#include "cube_light.h"

namespace Echo
{
	LightModule::LightModule()
	{

	}

	void LightModule::registerTypes()
	{
		Class::registerType<Light>();
		Class::registerType<CubeLightStatic>();
	}
}