#include "../camera_3d.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class Camera3DEditor : public ObjectEditor
	{
	public:
		Camera3DEditor(Object* object);

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;
	};
#endif
}