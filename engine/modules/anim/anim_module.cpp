#include "anim_module.h"
#include "anim_player.h"

namespace Echo
{
	AnimModule::AnimModule()
	{
	}

	void AnimModule::registerTypes()
	{
		Class::registerType<AnimPlayer>();
	}
}