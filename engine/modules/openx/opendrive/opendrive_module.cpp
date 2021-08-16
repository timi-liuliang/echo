#include "opendrive_module.h"

namespace Echo
{
	DECLARE_MODULE(OpenDriveModule)

	OpenDriveModule::OpenDriveModule()
	{
	}

	OpenDriveModule* OpenDriveModule::instance()
	{
		static OpenDriveModule* inst = EchoNew(OpenDriveModule);
		return inst;
	}

	void OpenDriveModule::bindMethods()
	{

	}

	void OpenDriveModule::registerTypes()
	{
	}
}