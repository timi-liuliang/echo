#include "anim_module.h"
#include "anim_player.h"
#include "editor/anim_player_editor.h"

namespace Echo
{
	AnimModule::AnimModule()
	{
	}

	void AnimModule::registerTypes()
	{
		Class::registerType<AnimPlayer>();

		REGISTER_OBJECT_EDITOR(AnimPlayer, AnimPlayerEditor)
	}
}