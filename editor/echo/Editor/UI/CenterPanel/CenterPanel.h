#pragma once

#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/editor/bottom_panel_tab.h>
#include <QDockWidget>
#include <QString>
#include "ui_CenterPanel.h"

namespace Studio
{
	class DebuggerPanel;
	class DocumentPanel;
	class CenterPanel : public QDockWidget, public Ui_CenterPanel
	{
		Q_OBJECT

	public:
		CenterPanel(QWidget* parent = 0);
		~CenterPanel();

		// show tab
		void setTabVisible(const Echo::String& tabName, bool isVisible);
		bool isTabVisible(const Echo::String& tabName);

		// show bottom panel
		void showPanel(Echo::PanelTab* panel, float widthRatio=0.75f, float heightRation=0.65f);

	public slots:
		// on tab index changed
		void onTabIdxChanged(int idx);

	private:
		// get tab
		QWidget* getTab(const Echo::String& tabName);

	private:
	};
}