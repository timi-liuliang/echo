#include "../audio_listener.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AudioListenerEditor : public ObjectEditor
	{
	public:
		AudioListenerEditor(Object* object);
		virtual ~AudioListenerEditor();

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;

	private:
	};

#endif
}