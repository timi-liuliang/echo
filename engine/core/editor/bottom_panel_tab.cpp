#include "bottom_panel_tab.h"
#include "editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	BottomPanelTab::BottomPanelTab()
		: m_ui(nullptr)
	{

	}

	BottomPanelTab::~BottomPanelTab()
	{
		EditorApi.qDeleteWidget(m_ui);
	}
#endif
}
