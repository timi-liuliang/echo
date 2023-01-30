#include "opendrive_module.h"
#include "opendrive.h"
#include "opendrive_debug_draw.h"
#include "opendrive_dynamic_mesh.h"
#include "editor/opendrive_editor.h"

namespace Echo
{
	DECLARE_MODULE(OpenDriveModule, __FILE__)

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
		Class::registerType<OpenDriveDebugDraw>();
		Class::registerType<OpenDriveDynamicMesh>();

		CLASS_REGISTER_EDITOR(OpenDrive, OpenDriveEditor)
	}
}