#include "scene_capture_module.h"
#include "scene_capture_3d.h"

namespace Echo
{
	DECLARE_MODULE(SceneCaptureModule, __FILE__)

	SceneCaptureModule::SceneCaptureModule()
	{

	}

	SceneCaptureModule* SceneCaptureModule::instance()
	{
		static SceneCaptureModule* inst = EchoNew(SceneCaptureModule);
		return inst;
	}

	void SceneCaptureModule::bindMethods()
	{

	}

	void SceneCaptureModule::registerTypes()
	{
		Class::registerType<SceneCapture>();
		Class::registerType<SceneCapture3D>();
	}
}