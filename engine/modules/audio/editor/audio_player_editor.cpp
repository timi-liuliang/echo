#include "audio_player_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	AudioPlayerEditor::AudioPlayerEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	AudioPlayerEditor::~AudioPlayerEditor()
	{
	}

	// get camera2d icon, used for editor
	const char* AudioPlayerEditor::getEditorIcon() const
	{
		return "engine/modules/audio/editor/icon/audioplayer.png";
	}
#endif
}