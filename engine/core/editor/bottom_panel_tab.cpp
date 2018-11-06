#include "bottom_panel_tab.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	BottomPanelTab::BottomPanelTab()
		: m_ui(nullptr)
	{

	}

	BottomPanelTab::~BottomPanelTab()
	{
		qDeleteWidget(m_ui);
	}
#endif
}