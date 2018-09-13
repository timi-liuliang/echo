#include "../camera_2d.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// get camera2d icon, used for editor
	const char* Camera2D::getEditorIcon() const
	{
		return "engine/modules/camera/editor/icon/camera2d.png";
	}

	// on editor select this node
	void Camera2D::onEditorSelectThisNode()
	{

	}
#endif
}