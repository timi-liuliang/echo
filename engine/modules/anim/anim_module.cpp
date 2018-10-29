#include "anim_module.h"
#include "anim_timeline.h"
#include "editor/anim_timeline_editor.h"

namespace Echo
{
	AnimModule::AnimModule()
	{
	}

	void AnimModule::registerTypes()
	{
		Class::registerType<Timeline>();

		REGISTER_OBJECT_EDITOR(Timeline, TimelineEditor)
	}
}