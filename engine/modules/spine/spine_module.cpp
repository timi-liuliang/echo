#include "spine_module.h"
#include "spine.h"

namespace Echo
{
	SpineModule::SpineModule()
	{
	}

	void SpineModule::registerTypes()
	{
		Class::registerType<Spine>();
	}
}