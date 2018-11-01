#pragma once

#include "engine/core/base/EchoDef.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class BottomPanelTab
	{
	public:
		BottomPanelTab();
		virtual ~BottomPanelTab() {}

		// get title
		virtual const char* getTitle() const { return "BottomPanelTab"; }

		// get ui file
		virtual const char* getUiFile() const { return ""; }

		// get ui ptr
		void* getUiPtr() { return m_ui; }

		// set ui ptr
		void setUiPtr(void* ui) { m_ui = ui; }

	public:
		void*		m_ui;
	};
}
#endif