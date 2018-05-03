#include "TimelinePanel.h"
#include "ui_TimelinePanel.h"
#include "QScrollBar"

namespace Studio
{
	TimelinePanel::TimelinePanel(QWidget *parent)
		: QWidget(parent)
	{
		setupUi(this);
	}

	TimelinePanel::~TimelinePanel()
	{
	}
}
