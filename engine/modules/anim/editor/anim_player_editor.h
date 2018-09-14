#include "../anim_player.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class AnimPlayerEditor : public ObjectEditor
	{
	public:
		AnimPlayerEditor(Object* object);

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;
	};

#endif
}