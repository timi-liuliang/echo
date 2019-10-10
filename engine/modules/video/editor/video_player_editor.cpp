#include "video_player_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	VideoPlayerEditor::VideoPlayerEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	VideoPlayerEditor::~VideoPlayerEditor()
	{
	}

	// get camera2d icon, used for editor
	const char* VideoPlayerEditor::getEditorIcon() const
	{
		return "engine/modules/video/editor/icon/videoplayer.png";
	}
#endif
}