#include "audio_listener_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	AudioListenerEditor::AudioListenerEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	AudioListenerEditor::~AudioListenerEditor()
	{
	}

	// get camera2d icon, used for editor
	const char* AudioListenerEditor::getEditorIcon() const
	{
		return "engine/modules/audio/editor/icon/audiolistener.png";
	}
#endif
}