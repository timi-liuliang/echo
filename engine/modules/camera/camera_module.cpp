#include "camera_module.h"
#include "camera_2d.h"

namespace Echo
{
	CameraModule::CameraModule()
	{

	}

	void CameraModule::registerTypes()
	{
		Class::registerType<Camera2D>();
	}
}