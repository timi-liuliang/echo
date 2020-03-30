#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// get open file names
	typedef bool (*qDialogExecFun)(QWidget* widget);
}

#endif
