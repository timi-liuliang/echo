#pragma once

#include "engine/core/base/object.h"

#ifdef ECHO_EDITOR_MODE

#include <QWidget>
#include <QTreeWidgetItem>

namespace Echo
{
	class QTimelineKeyFrameBar : public QWidget
	{
	public:
		QTimelineKeyFrameBar(QTreeWidgetItem* parent=nullptr);

	protected:
		// Override
		virtual void paintEvent(QPaintEvent* event);

	protected:
		QTreeWidgetItem* m_parent = nullptr;
	};
}

#endif