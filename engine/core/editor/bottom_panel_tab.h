#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class BottomPanelTab
	{
	public:
		// get title
		virtual const char* getTitle() const { return "BottomPanelTab"; }

		// get ui file
		virtual const char* getUiFile() const { return ""; }
	};
}
#endif