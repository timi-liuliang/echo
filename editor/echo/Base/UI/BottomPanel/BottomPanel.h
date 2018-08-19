#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <QDockWidget>
#include <QString>
#include "ui_BottomPanel.h"

namespace Studio
{
	class TimelinePanel;
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

	public slots:
		// on tab index changed
		void onTabIdxChanged(int idx);

	private:
		DocumentPanel*	m_documentPanel;
		TimelinePanel*	m_timelinePanel;
		DebuggerPanel*	m_debuggerPanel;
	};
}