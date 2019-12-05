#include "editor_settings_module.h"
#include "editor_settings.h"
#include "editor_camera_settings.h"

namespace Echo
{
	DECLARE_MODULE(EditorSettingsModule)

	EditorSettingsModule::EditorSettingsModule()
    {
    }

	EditorSettingsModule::~EditorSettingsModule()
    {

    }

	EditorSettingsModule* EditorSettingsModule::instance()
	{
		static EditorSettingsModule* inst = EchoNew(EditorSettingsModule);
		return inst;
	}

	void EditorSettingsModule::bindMethods()
	{

	}

    void EditorSettingsModule::registerTypes()
    {
        Class::registerType<EditorSettings>();
        Class::registerType<EditorCameraSettings>();
    }
}
