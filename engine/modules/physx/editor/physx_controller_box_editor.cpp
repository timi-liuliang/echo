#include "physx_controller_box_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PhysxControllerBoxEditor::PhysxControllerBoxEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	PhysxControllerBoxEditor::~PhysxControllerBoxEditor()
	{
	}

	ImagePtr PhysxControllerBoxEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/physx/editor/icon/physx_controller_box.png");
	}

	void PhysxControllerBoxEditor::onEditorSelectThisNode()
	{
	}

	void PhysxControllerBoxEditor::editor_update_self()
	{
	}
#endif
}

