#include "bottom_panel_tab.h"
#include "editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	PanelTab::PanelTab()
		: m_ui(nullptr)
	{

	}

	PanelTab::~PanelTab()
	{
		delete m_ui; m_ui = nullptr;
	}
#endif
}
