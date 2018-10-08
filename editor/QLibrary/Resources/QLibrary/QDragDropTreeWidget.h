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
		void itemPositionChanged(QTreeWidgetItem* item);

	protected:
		// drag move event
		virtual void dragMoveEvent(QDragMoveEvent* event) override;

		// override drop event
		virtual void dropEvent(QDropEvent *event) override;
	};
}