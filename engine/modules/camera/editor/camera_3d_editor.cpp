#include "camera_3d_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	REGISTER_OBJECT_EDITOR(Camera3D, Camera3DEditor)

	Camera3DEditor::Camera3DEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* Camera3DEditor::getEditorIcon() const
	{
		return "engine/modules/camera/editor/icon/camera3d.png";
	}
#endif
}