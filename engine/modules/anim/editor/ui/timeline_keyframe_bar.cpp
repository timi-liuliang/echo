#include "timeline_keyframe_bar.h"

#ifdef ECHO_EDITOR_MODE

#include <QPen>
#include <QPainter>

namespace Echo
{
	QTimelineKeyFrameBar::QTimelineKeyFrameBar(QTreeWidgetItem* parent)
		: QWidget(nullptr)
		, m_parent(parent)
	{

	}

	void QTimelineKeyFrameBar::paintEvent(QPaintEvent* event)
	{
        QPainter painter(this);
        painter.fillRect(this->rect(), m_parent->isSelected() ? QColor(14, 99, 156) : QColor(77, 77, 77, 255));
	}
}

#endif