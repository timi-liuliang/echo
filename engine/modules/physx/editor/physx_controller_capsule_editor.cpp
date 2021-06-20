#include "physx_controller_capsule_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxControllerCapsuleEditor::PhysxControllerCapsuleEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	PhysxControllerCapsuleEditor::~PhysxControllerCapsuleEditor()
	{
	}

	ImagePtr PhysxControllerCapsuleEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_controller_capsule.png");
	}

	void PhysxControllerCapsuleEditor::onEditorSelectThisNode()
	{
	}
#endif
}

