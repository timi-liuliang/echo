#include "physx_body_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxBodyEditor::PhysxBodyEditor(Object* object)
		: ObjectEditor(object)
	{
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
	}

	PhysxBodyEditor::~PhysxBodyEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
	}

	ImagePtr PhysxBodyEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_body.png");
	}

	void PhysxBodyEditor::onEditorSelectThisNode()
	{
	}

	void PhysxBodyEditor::editor_update_self()
	{
		m_gizmo->clear();
		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

