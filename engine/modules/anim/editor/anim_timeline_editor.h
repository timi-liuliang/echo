#include "../anim_timeline.h"
#include "TimelinePanel/TimelinePanel.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class TimelineEditor : public ObjectEditor
	{
	public:
		TimelineEditor(Object* object);
		virtual ~TimelineEditor();

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

	private:
		TimelinePanel*		m_timelinePanel;
	};

#endif
}