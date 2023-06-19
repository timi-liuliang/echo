#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>

namespace Echo
{
	class QTimelinePropertyBar : public QWidget
	{
	public:
		QTimelinePropertyBar(QWidget* parent = nullptr);
	};
}

#endif