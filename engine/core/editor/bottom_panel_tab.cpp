#include "bottom_panel_tab.h"

namespace Echo
{
	BottomPanelTab::BottomPanelTab()
		: m_ui(nullptr)
	{

	}

	BottomPanelTab::~BottomPanelTab()
	{
		qDeleteWidget(m_ui);
	}
}