#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// set tool button fixed width
	void qToolButtonSetFixedWidth(QWidget* button, int width);

	// set tool button icon
	void qToolButtonSetIcon(QWidget* button, const char* iconPath);
}

#endif