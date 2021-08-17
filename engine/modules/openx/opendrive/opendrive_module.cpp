#include "opendrive_module.h"
#include "opendrive.h"
#include "editor/opendrive_editor.h"

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
		Class::registerType<OpenDrive>();

		CLASS_REGISTER_EDITOR(OpenDrive, OpenDriveEditor)
	}
}