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
		m_gizmo = ECHO_DOWN_CAST<Echo::Gizmos*>(Echo::Class::create("Gizmos"));
		m_gizmo->setName(StringUtil::Format("gizmo_obj_%d", m_object->getId()));
	}

	RvoAgentEditor::~RvoAgentEditor()
	{
		EchoSafeDelete(m_gizmo, Gizmos);
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
		m_gizmo->clear();

		RvoAgent* agent = ECHO_DOWN_CAST<RvoAgent*>(m_object);
		if (agent)
		{
			m_gizmo->setRenderType("3d");
			m_gizmo->drawCircle(agent->getWorldPosition(), Vector3::UNIT_Y, agent->getRadius(), Color::WHITE);
		}

		m_gizmo->update(Engine::instance()->getFrameTime(), true);
	}
#endif
}

