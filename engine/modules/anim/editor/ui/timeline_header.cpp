#include "timeline_header.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	QTimelineHeader::QTimelineHeader(Qt::Orientation orientation, QWidget* parent)
		: QHeaderView(orientation, parent) 
	{
	}

	void QTimelineHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
	{
		QHeaderView::paintSection(painter, rect, logicalIndex);
	}
}

#endif