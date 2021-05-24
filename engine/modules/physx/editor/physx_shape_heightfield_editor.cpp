#include "physx_shape_heightfield_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxShapeHeightfieldEditor::PhysxShapeHeightfieldEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
	}

	PhysxShapeHeightfieldEditor::~PhysxShapeHeightfieldEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr PhysxShapeHeightfieldEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_shape_heightfield.png");
	}

	void PhysxShapeHeightfieldEditor::onEditorSelectThisNode()
	{
	}

	void PhysxShapeHeightfieldEditor::editor_update_self()
	{
		m_gizmo->clear();
		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

