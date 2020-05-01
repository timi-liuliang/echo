#pragma once

#include "../audio_listener.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AudioListenerEditor : public ObjectEditor
	{
	public:
		AudioListenerEditor(Object* object);
		virtual ~AudioListenerEditor();

		// get icon, used for editor
        virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}
