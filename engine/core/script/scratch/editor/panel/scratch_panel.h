#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ScratchEditorPanel
	{
	public:
		ScratchEditorPanel(Object* obj);

		// get ui
		QDockWidget* getUi() { return m_ui; }

		// update
		void update();

	private:
		QDockWidget* m_ui = nullptr;
	};
}

#endif