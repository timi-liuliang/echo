#include "timeline_header.h"

#ifdef ECHO_EDITOR_MODE

#include <QPen>

namespace Echo
{
	QTimelineHeader::QTimelineHeader(Qt::Orientation orientation, QWidget* parent)
		: QHeaderView(orientation, parent) 
	{

	}

	void QTimelineHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
	{
		if (logicalIndex == 2)
		{
			paintSectionRuler(painter, rect);
		}
		else
		{
			QHeaderView::paintSection(painter, rect, logicalIndex);
		}
	}

	void QTimelineHeader::paintSectionRuler(QPainter* painter, const QRect& rect) const
	{
		m_ruler.draw(*painter, rect);
	}
}

#endif