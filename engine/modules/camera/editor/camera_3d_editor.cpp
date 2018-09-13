#include "../camera_3d.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// get camera2d icon, used for editor
	const char* Camera3D::getEditorIcon() const
	{
		return "engine/modules/camera/editor/icon/camera3d.png";
	}

	// on editor select this node
	void Camera3D::onEditorSelectThisNode()
	{

	}
#endif
}