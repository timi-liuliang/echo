#include "live2d_module.h"
#include "live2d_cubism.h"

extern "C"
{
	#include "thirdparty\live2d\Cubism31SdkNative-EAP5\Core\include\Live2DCubismCore.h"
}

namespace Echo
{
	// log function for cubism
	static void log_cb(const char* message)
	{

	}

	Live2DModule::Live2DModule()
	{
		// set log callback function
		csmSetLogFunction(log_cb);
	}

	void Live2DModule::registerTypes()
	{
		Class::registerType<Live2dCubism>();
	}
}