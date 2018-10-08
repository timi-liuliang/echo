#pragma once

#include <QTreeWidget>

namespace QT_UI
{
	class QDragDropTreeWidget : public QTreeWidget
	{
		Q_OBJECT

	public:
		QDragDropTreeWidget(QWidget* parent);

	protected:
		// override drop event
		virtual void dropEvent(QDropEvent *event) override;
	};
}