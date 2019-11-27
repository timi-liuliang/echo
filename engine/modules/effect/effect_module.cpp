#include "effect_module.h"
#include "sprite.h"

namespace Echo
{
	DECLARE_MODULE(EffectModule)

	EffectModule::EffectModule()
	{
	}

	EffectModule* EffectModule::instance()
	{
		static EffectModule* inst = EchoNew(EffectModule);
		return inst;
	}

	void EffectModule::bindMethods()
	{

	}

	void EffectModule::registerTypes()
	{
		Class::registerType<Sprite>();
	}
}