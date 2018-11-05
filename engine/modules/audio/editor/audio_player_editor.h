#include "../audio_player.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AudioPlayerEditor : public ObjectEditor
	{
	public:
		AudioPlayerEditor(Object* object);
		virtual ~AudioPlayerEditor();

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;

	private:
	};

#endif
}