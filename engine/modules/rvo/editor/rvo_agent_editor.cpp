#include "rvo_agent_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Curve.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	RvoAgentEditor::RvoAgentEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	RvoAgentEditor::~RvoAgentEditor()
	{

	}

	ImagePtr RvoAgentEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/rvo/editor/icon/rvo-agent.png");
	}

	void RvoAgentEditor::onEditorSelectThisNode()
	{
	}

	void RvoAgentEditor::postEditorCreateObject()
	{

	}

	void RvoAgentEditor::editor_update_self()
	{

	}
#endif
}

