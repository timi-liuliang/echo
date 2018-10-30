#pragma once

#include "engine/core/editor/bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class TimelinePanel : public BottomPanelTab
	{
	public:
		// get title
		virtual const char* getTitle() const override;

		// get ui file
		virtual const char* getUiFile() const override;
	};
}

#endif