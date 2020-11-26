#include "QResListView.h"
#include <QDragEnterEvent>

namespace QT_UI
{
	QResListView::QResListView(QWidget* parent)
		: QListView(parent)
	{
	}

	QResListView::~QResListView()
	{

	}

	void QResListView::dragEnterEvent(QDragEnterEvent* event)
	{
		event->acceptProposedAction();
	}

	void QResListView::dragMoveEvent(QDragMoveEvent* event)
	{
		event->acceptProposedAction();
	}

	void QResListView::dragLeaveEvent(QDragLeaveEvent* event)
	{
	}

	void QResListView::dropEvent(QDropEvent* event)
	{
		event->acceptProposedAction();
	}
}