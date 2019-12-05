#include "editor_camera_settings.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/main/Engine.h>
#include <thirdparty/pugixml/pugixml.hpp>
#include <QProcess>
#include "FreeImageHelper.h"
#include <engine/core/main/module.h>

namespace Echo
{
	EditorCameraSettings::EditorCameraSettings()
    {
    }

	EditorCameraSettings::~EditorCameraSettings()
    {

    }

	EditorCameraSettings* EditorCameraSettings::instance()
    {
        static EditorCameraSettings* inst = EchoNew(EditorCameraSettings);
        return inst;
    }

    void EditorCameraSettings::bindMethods()
    {
		CLASS_BIND_METHOD(EditorCameraSettings, getMoveSpeed, DEF_METHOD("getMoveSpeed"));
		CLASS_BIND_METHOD(EditorCameraSettings, setMoveSpeed, DEF_METHOD("setMoveSpeed"));

		CLASS_REGISTER_PROPERTY(EditorCameraSettings, "MoveSpeed", Variant::Type::Real, "getMoveSpeed", "setMoveSpeed");

		//CLASS_REGISTER_PROPERTY_HINT(EditorCameraSettings, "MoveSpeed", PropertyHintType::Category, "3D Camera");
    }

	void EditorCameraSettings::setMoveSpeed(float moveSpeed)
	{ 


		m_moveSpeed = moveSpeed; 
	}
}
