#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>
#include <QTreeWidgetItem>
#include "ui_timeline_object_bar.h"

namespace Echo
{
	class QTimelineObjectBar : public QWidget, public Ui_TimelineObjectBar
	{
	public:
		QTimelineObjectBar(QTreeWidgetItem* parent = nullptr);

		// Set tool bar icon
		void setToolbuttonIcon(QToolButton* button, const String& iconPath);

		// Set text
		void setText(QString text);
	};
}

#endif