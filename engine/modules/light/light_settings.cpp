#include "light_settings.h"

namespace Echo
{
	LightSettings::LightSettings()
	{

	}

	LightSettings::~LightSettings()
	{

	}

	void LightSettings::bindMethods()
	{

	}

	LightSettings* LightSettings::instance()
	{
		static LightSettings* inst = EchoNew(LightSettings);
		return inst;
	}
}