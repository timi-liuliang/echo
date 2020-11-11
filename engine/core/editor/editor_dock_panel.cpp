#include "editor_dock_panel.h"
#include "editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	EditorDockPanel::EditorDockPanel()
		: m_ui(nullptr)
	{

	}

	EditorDockPanel::~EditorDockPanel()
	{
		delete m_ui; m_ui = nullptr;
	}
#endif
}
