#include "physx_vehicle_chassis_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"
#include "../vehicle/physx_vehicle_chassis.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxVehicleChassisEditor::PhysxVehicleChassisEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("3d");
	}

	PhysxVehicleChassisEditor::~PhysxVehicleChassisEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr PhysxVehicleChassisEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_vehicle_chassis.png");
	}

	void PhysxVehicleChassisEditor::onEditorSelectThisNode()
	{
	}

	void PhysxVehicleChassisEditor::editor_update_self()
	{
		m_gizmo->clear();

		PhysxVehicleChassis* chassis = ECHO_DOWN_CAST<PhysxVehicleChassis*>(m_object);
		if (chassis)
		{
			Vector3 center = chassis->getWorldPosition();
			Matrix4 transform = chassis->getWorldMatrix();
			Vector3 dims = chassis->getDims();

			m_gizmo->drawOBB(dims, transform, Color::PURPLE);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

