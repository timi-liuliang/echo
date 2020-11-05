#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/bottom_panel_tab.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ScratchEditorPanel : public PanelTab
	{
	public:
		ScratchEditorPanel(Object* obj);

		// update
		void update();
	};
}

#endif