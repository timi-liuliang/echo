#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "ui_scratch_panel.h"

namespace Echo
{
	class ScratchEditorPanel : public QDockWidget, Ui_ScratchPanel
	{
	public:
		ScratchEditorPanel(Object* obj);

		// update
		void update();
	};
}

#endif