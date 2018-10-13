#include "light_module.h"
#include "light.h"
#include "cube_light_custom.h"
#include "cube_light_capture.h"

namespace Echo
{
	LightModule::LightModule()
	{

	}

	void LightModule::registerTypes()
	{
		Class::registerType<Light>();
		Class::registerType<CubeLightCustom>();
		Class::registerType<CubeLightCapture>();
	}
}