#include "anim_player_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	AnimPlayerEditor::AnimPlayerEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* AnimPlayerEditor::getEditorIcon() const
	{
		return "engine/modules/anim/editor/icon/animplayer.png";
	}
#endif
}