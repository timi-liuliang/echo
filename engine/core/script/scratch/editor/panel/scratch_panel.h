#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class ScratchEditorPanel : public EditorDockPanel
	{
	public:
		ScratchEditorPanel(Object* obj);

		// update
		void update();
	};
}

#endif