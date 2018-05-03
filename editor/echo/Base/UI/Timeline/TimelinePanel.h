#pragma once

#include <QWidget>
#include <QScopedPointer>
#include "ui_TimelinePanel.h"

namespace Studio
{
	class TimelinePanel : public QWidget, Ui_TimelinePanel
	{
		Q_OBJECT

	public:
		TimelinePanel(QWidget* parent = 0);
		~TimelinePanel();
	};
}
