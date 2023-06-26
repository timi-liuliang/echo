#include "timeline_header.h"
#include "timeline_panel.h"

#ifdef ECHO_EDITOR_MODE

#include <QPen>

namespace Echo
{
	QTimelineHeader::QTimelineHeader(Qt::Orientation orientation, TimelinePanel* timelinePanel)
		: QHeaderView(orientation, timelinePanel)
		, m_timelinePanel(timelinePanel)
	{
		m_toolBar = new QTimelineHeaderToolBar(this);
		m_toolBar->installEventFilter(this);
		m_toolBar->hide();
	}

	void QTimelineHeader::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
	{
		if (logicalIndex == 0)
		{
			m_toolBar->setGeometry(rect);
			m_toolBar->show();
		}
		else if (logicalIndex == 2)
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


	bool QTimelineHeader::eventFilter(QObject* obj, QEvent* event) 
	{
		if (obj == m_toolBar && event->type() == QEvent::MouseButtonPress)
		{
			return true;
		}

		return false;
	}
}

#endif