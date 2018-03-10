#pragma once

#include <QListWidget>

class QWheelEvent;

namespace Studio
{
	class ListWidget : public QListWidget
	{
		Q_OBJECT
	public:
		ListWidget(QWidget* parent = 0);
		~ListWidget();

	signals:
		void verticalScroll(int val);

	};

}  // namespace Studio
