#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/editor/bottom_panel_tab.h>
#include <QDockWidget>
#include <QString>
#include "ui_BottomPanel.h"

namespace Studio
{
	class DebuggerPanel;
	class DocumentPanel;
	class BottomPanel : public QDockWidget, public Ui_BottomPanel
	{
		Q_OBJECT

	public:
		BottomPanel(QWidget* parent = 0);
		~BottomPanel();

		// show tab
		void setTabVisible(const Echo::String& tabName, bool isVisible);
		bool isTabVisible(const Echo::String& tabName);

		// show bottom panel
		void showBottomPanel(Echo::BottomPanelTab* bottomPanel);

	public slots:
		// on tab index changed
		void onTabIdxChanged(int idx);

	private:
		// get tab
		QWidget* getTab(const Echo::String& tabName);

	private:
		DocumentPanel*	m_documentPanel;
		DebuggerPanel*	m_debuggerPanel;
	};
}