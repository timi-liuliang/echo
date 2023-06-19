#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>

namespace Echo
{
	class QTimelineObjectBar : public QWidget
	{
	public:
		QTimelineObjectBar(QWidget* parent = nullptr);
	};
}

#endif