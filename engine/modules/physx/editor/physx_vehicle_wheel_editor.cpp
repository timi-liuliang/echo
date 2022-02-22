#include "physx_vehicle_wheel_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"
#include "../vehicle/physx_vehicle_wheel.h"
#include "../physx_module.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxVehicleWheelEditor::PhysxVehicleWheelEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
		m_gizmo->setRenderType("3d");
	}

	PhysxVehicleWheelEditor::~PhysxVehicleWheelEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr PhysxVehicleWheelEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_vehicle_wheel.png");
	}

	void PhysxVehicleWheelEditor::onEditorSelectThisNode()
	{
	}

	void PhysxVehicleWheelEditor::editor_update_self()
	{
		m_gizmo->clear();

		PhysxVehicleWheel* wheel = ECHO_DOWN_CAST<PhysxVehicleWheel*>(m_object);
		if (wheel)
		{
			Vector3 center = wheel->getWorldPosition();
			Vector3 up	   = wheel->getWorldOrientation().rotateVec3(Vector3::UNIT_Z);
			float   radius = wheel->getRadius();
			float   height = wheel->getWidth();

			m_gizmo->drawCylinder(center, up, height, radius, Color::PURPLE, 16);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

