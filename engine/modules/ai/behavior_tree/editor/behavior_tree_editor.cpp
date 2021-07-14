#include "behavior_tree_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	BehaviorTreeEditor::BehaviorTreeEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	BehaviorTreeEditor::~BehaviorTreeEditor()
	{
	}

	ImagePtr BehaviorTreeEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/ai/behavior_tree/editor/icon/behavior_tree.png");
	}

	void BehaviorTreeEditor::onEditorSelectThisNode()
	{
	}

	void BehaviorTreeEditor::editor_update_self()
	{
	}
#endif
}

