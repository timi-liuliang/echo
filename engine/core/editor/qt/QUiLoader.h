#pragma once

#include "engine/core/base/EchoDef.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// load ui file by path. then return the widget
	void* qLoadUiFile(const char* uiFilePath);
#endif
}