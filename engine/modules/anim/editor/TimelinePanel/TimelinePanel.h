#pragma once

#include "engine/core/editor/bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class Timeline;
	class TimelinePanel : public BottomPanelTab
	{
	public:
		TimelinePanel(Object* obj);

	public:
		// on new clip
		void onNewClip();

	private:
		// get new name
		String getNewClipName();

		// sync clip data to editor
		void syncClipDataToEditor();

	protected:
		Timeline*		m_timeline;
	};
}

#endif