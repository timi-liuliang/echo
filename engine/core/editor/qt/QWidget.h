#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// find child widget by name
	QWidget* qFindChild( QWidget* root, const char* name);

	// delete QWidget
	void qDeleteWidget(QWidget*& widget);
}

#endif