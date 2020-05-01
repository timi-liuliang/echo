#include "audio_player_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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

	ImagePtr AudioPlayerEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/audio/editor/icon/audioplayer.png");
	}
#endif
}
