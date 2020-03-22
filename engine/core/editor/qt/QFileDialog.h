#pragma once

#include "QWidget.h"

#ifdef ECHO_EDITOR_MODE

class QWidget;

namespace Echo
{
	// get open file names
	typedef StringArray(*qGetOpenFileNamesFun)(QWidget* parent, const String& caption, const String& dir, const String& filer);
}

#endif
