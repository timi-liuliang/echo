#pragma once

#include <QTreeWidget>
#include <QDropEvent>

namespace QT_UI
{
	class QDragDropTreeWidget : public QTreeWidget
	{
		Q_OBJECT

	public:
		QDragDropTreeWidget(QWidget* parent);

	signals:
		// drop signal
		void itemChildrenChanged(QTreeWidgetItem* item);

	protected:
		// override drop event
		virtual void dropEvent(QDropEvent *event) override;
	};
}