#pragma once

#include "engine/core/editor/qt/QWidget.h"

#ifdef ECHO_EDITOR_MODE

namespace Echo
{
	class PanelTab
	{
	public:
		PanelTab();
		virtual ~PanelTab();

		// get ui ptr
		QDockWidget* getUiPtr() { return m_ui; }

		// save
		virtual void save() {}

	public:
		QDockWidget*		m_ui;
	};
}
#endif