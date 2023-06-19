#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>

namespace Echo
{
	class QTimelineHeaderToolBar : public QWidget
	{
	public:
		QTimelineHeaderToolBar(QWidget* parent = nullptr);
	};
}

#endif