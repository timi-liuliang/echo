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
		QWidget* getUiPtr() { return m_ui; }

	public:
		QWidget*		m_ui;
	};
}
#endif