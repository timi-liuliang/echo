#include "editor_general_settings.h"

namespace Echo
{
    EditorGeneralSettings::EditorGeneralSettings()
    {
    }

    EditorGeneralSettings::~EditorGeneralSettings()
    {

    }

    EditorGeneralSettings* EditorGeneralSettings::instance()
    {
        static EditorGeneralSettings* inst = EchoNew(EditorGeneralSettings);
        return inst;
    }

    void EditorGeneralSettings::bindMethods()
    {
		CLASS_BIND_METHOD(EditorGeneralSettings, isShow3dGrid, DEF_METHOD("isShow3dGrid"));
		CLASS_BIND_METHOD(EditorGeneralSettings, setShow3dGrid, DEF_METHOD("setShow3dGrid"));

		CLASS_REGISTER_PROPERTY(EditorGeneralSettings, "Show3dGrid", Variant::Type::Bool, "isShow3dGrid", "setShow3dGrid");
    }
}
