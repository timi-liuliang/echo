#pragma once

#include "engine/core/base/echo_def.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// set tool button fixed width
	typedef void (*qToolButtonSetFixedWidthFun)(QWidget* button, int width);

	// set tool button icon
	typedef void (*qToolButtonSetIconFun)(QWidget* button, const char* iconPath);

	// set icon size
	typedef void (*qToolButtonSetIconSizeFun)(QWidget* widget, int width, int height);

	// is toolbutton checked
	typedef bool (*qToolButtonIsCheckedFun)(QWidget* widget);
}

#endif
