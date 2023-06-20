#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>
#include "ui_timeline_header_tool_bar.h"

namespace Echo
{
	class QTimelineHeaderToolBar : public QWidget, public Ui_TimelineHeaderToolBar
	{
	public:
		QTimelineHeaderToolBar(QWidget* parent = nullptr);

		// Set tool button icon
		void setToolbuttonIcon(QToolButton* button, const String& iconPath);

	private:

	};
}

#endif