#pragma once

#include "QWidget.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// load ui file by path. then return the widget
	typedef QWidget* (*qLoadUiFun)(const char* uiFilePath);
#endif
}
