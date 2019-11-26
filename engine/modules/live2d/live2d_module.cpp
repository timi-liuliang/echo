#include "engine/core/log/Log.h"
#include "live2d_module.h"
#include "live2d_cubism.h"
#include "live2d_cubism_motion_res.h"

namespace Echo
{
	// log function for cubism
	static void log_cb(const char* message)
	{
		EchoLogInfo(message);
	}

	Live2DModule::Live2DModule()
	{
		// set log callback function
		csmSetLogFunction(log_cb);
	}

	Live2DModule* Live2DModule::instance()
	{
		static Live2DModule* inst = EchoNew(Live2DModule);
		return inst;
	}

	void Live2DModule::bindMethods()
	{

	}

	void Live2DModule::registerTypes()
	{
		Class::registerType<Live2dCubism>();
	}
}