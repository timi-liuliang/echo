#include "effect_module.h"
#include "sprite.h"

namespace Echo
{
	EffectModule::EffectModule()
	{
	}

	void EffectModule::registerTypes()
	{
		Class::registerType<Sprite>();
	}
}