#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_BottomPanel.h"
#include <Engine/Core.h>
#include <engine/core/Util/Log.h>

namespace Studio
{
	class TimelinePanel;
	class DebuggerPanel;
	class BottomPanel : public QDockWidget, public Ui_BottomPanel
	{
		Q_OBJECT

	public:
		// 构造函数
		BottomPanel(QWidget* parent = 0);

		// 析构函数
		~BottomPanel();

	private:
		TimelinePanel*	m_timelinePanel;
		DebuggerPanel*	m_debuggerPanel;
	};
}