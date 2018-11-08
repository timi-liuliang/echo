#include "anim_timeline_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TimelineEditor::TimelineEditor(Object* object)
		: ObjectEditor(object)
		, m_timelinePanel(nullptr)
	{
		m_timelinePanel = EchoNew(TimelinePanel(object));
	}

	TimelineEditor::~TimelineEditor()
	{
		EchoSafeDelete(m_timelinePanel, TimelinePanel);
	}

	// get camera2d icon, used for editor
	const char* TimelineEditor::getEditorIcon() const
	{
		return "engine/modules/anim/editor/icon/animtimeline.png";
	}

	// on editor select this node
	void TimelineEditor::onEditorSelectThisNode()
	{
		Editor::instance()->showBottomPanel(m_timelinePanel);
	}
#endif
}