#include "effect_module.h"
#include "sprite_2d.h"

namespace Echo
{
	EffectModule::EffectModule()
	{
	}

	void EffectModule::registerTypes()
	{
		Class::registerType<Sprite2D>();
	}
}