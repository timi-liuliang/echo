#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// set tool button icon
	void qToolButtonSetIcon(QWidget* button, const char* iconPath);
}

#endif