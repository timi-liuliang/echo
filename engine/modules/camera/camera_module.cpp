#include "camera_module.h"
#include "camera_2d.h"
#include "camera_3d.h"
#include "editor/camera_2d_editor.h"
#include "editor/camera_3d_editor.h"

namespace Echo
{
	CameraModule::CameraModule()
	{

	}

	void CameraModule::registerTypes()
	{
		Class::registerType<Camera2D>();
		Class::registerType<Camera3D>();

		REGISTER_OBJECT_EDITOR(Camera2D, Camera2DEditor)
		REGISTER_OBJECT_EDITOR(Camera3D, Camera3DEditor)
	}
}