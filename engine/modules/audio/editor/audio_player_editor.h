#pragma once

#include "../audio_player.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AudioPlayerEditor : public ObjectEditor
	{
	public:
		AudioPlayerEditor(Object* object);
		virtual ~AudioPlayerEditor();

		// get icon, used for editor
		virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
