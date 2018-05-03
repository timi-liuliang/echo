#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_DebuggerPanel.h"

namespace Studio
{
	//----------------------------------------
	// 输出 主窗口 2010-12-30
	//----------------------------------------
	class DebuggerPanel : public QWidget, public Ui_DebuggerPanel
	{
		Q_OBJECT

	public:
		// 构造函数
		DebuggerPanel(QWidget* parent = 0);

		// 析构函数
		~DebuggerPanel();

	protected:
		// 关闭
		//void closeEvent(QCloseEvent *event);
	};
}