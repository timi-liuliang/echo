#include "physx_vehicle_drive4w_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxVehicleDrive4WEditor::PhysxVehicleDrive4WEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
	}

	PhysxVehicleDrive4WEditor::~PhysxVehicleDrive4WEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr PhysxVehicleDrive4WEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_vehicle_drive4w.png");
	}

	void PhysxVehicleDrive4WEditor::onEditorSelectThisNode()
	{
	}

	void PhysxVehicleDrive4WEditor::editor_update_self()
	{
		m_gizmo->clear();
		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

