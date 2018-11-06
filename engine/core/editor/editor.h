#pragma once

#include "engine/core/base/echo_def.h"
#include "bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class Editor
	{
	public:
		Editor() {}
		virtual ~Editor() {}

		// get instance
		static Editor* instance();

		// set instance
		static void setInstance(Editor* inst);

	public:
		// show bottom panel
		virtual void showBottomPanel(BottomPanelTab* bottomPanel)=0;
	};
}
#endif