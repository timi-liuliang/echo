#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get cursor current pos
	typedef Vector2 (*qCursorPosFun)();
}

#endif
