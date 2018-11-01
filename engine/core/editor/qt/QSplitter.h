#pragma once

#include "engine/core/editor/qt/QWidget.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// modify qsplitter stretch factor
	void qSplitterSetStretchFactor(QWidget* widget, int index, int stretch);
#endif
}
