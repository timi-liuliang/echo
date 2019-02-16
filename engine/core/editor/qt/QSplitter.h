#pragma once

#include "engine/core/editor/qt/QWidget.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// modify qsplitter stretch factor
	typedef void (*qSplitterSetStretchFactorFun)(QWidget* widget, int index, int stretch);
#endif
}
