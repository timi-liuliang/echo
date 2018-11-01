#pragma once

#include "engine/core/editor/qt/QWidget.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class BottomPanelTab
	{
	public:
		BottomPanelTab();
		virtual ~BottomPanelTab();

		// get ui ptr
		QWidget* getUiPtr() { return m_ui; }

	public:
		QWidget*		m_ui;
	};
}
#endif