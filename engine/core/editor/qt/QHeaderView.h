#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	enum QHeaderViewResizeMode
	{
		Interactive,
		Stretch,
		Fixed,
		ResizeToContents,
		Custom = Fixed
	};

	// resize section
	void qHeaderViewResizeSection(QWidget* header, int logicalIndex, int size);

	// set section size mode
	void qHeaderViewSetSectionResizeMode(QWidget* header, int logicalIndex, int mode);
}

#endif