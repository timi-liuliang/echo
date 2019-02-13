#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	// new QCheckBox
	typedef QWidget* (*qCheckBoxNewFun)();
}

#endif
