#include "camera_module.h"
#include "camera_2d.h"
#include "camera_3d.h"
#include "editor/camera_2d_editor.h"
#include "editor/camera_3d_editor.h"

namespace Echo
{
	DECLARE_MODULE(CameraModule, __FILE__)

	CameraModule::CameraModule()
	{

	}

	CameraModule* CameraModule::instance()
	{
		static CameraModule* inst = EchoNew(CameraModule);
		return inst;
	}

	void CameraModule::bindMethods()
	{

	}

	void CameraModule::registerTypes()
	{
		Class::registerType<Camera2D>();
		Class::registerType<Camera3D>();

		CLASS_REGISTER_EDITOR(Camera2D, Camera2DEditor)
		CLASS_REGISTER_EDITOR(Camera3D, Camera3DEditor)
	}
}