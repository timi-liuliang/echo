#include "audio_listener_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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

	ImagePtr AudioListenerEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/audio/editor/icon/audiolistener.png");
	}
#endif
}
