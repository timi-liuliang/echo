#include "spine_module.h"
#include "spine.h"

namespace Echo
{
	DECLARE_MODULE(SpineModule)

	SpineModule::SpineModule()
	{
	}

	SpineModule* SpineModule::instance()
	{
		static SpineModule* inst = EchoNew(SpineModule);
		return inst;
	}

	void SpineModule::bindMethods()
	{

	}

	void SpineModule::registerTypes()
	{
		Class::registerType<Spine>();
	}
}