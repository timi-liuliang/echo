#pragma once

#include <QListView>
#include <QDrag>
#include <QMimeData>

namespace QT_UI
{
	class QResListView : public QListView
	{
		Q_OBJECT

	public:
		QResListView(QWidget* parent);
		virtual ~QResListView();

		// drag drop
		virtual void dragEnterEvent(QDragEnterEvent* event) override;
		virtual void dragMoveEvent(QDragMoveEvent* event) override;
		virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
		virtual void dropEvent(QDropEvent* event) override;

	protected:
	};
}