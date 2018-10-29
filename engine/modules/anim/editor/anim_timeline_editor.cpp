#include "anim_timeline_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TimelineEditor::TimelineEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* TimelineEditor::getEditorIcon() const
	{
		return "engine/modules/anim/editor/icon/animtimeline.png";
	}

	// on editor select this node
	void TimelineEditor::onEditorSelectThisNode()
	{
		int a = 10;
	}
#endif
}