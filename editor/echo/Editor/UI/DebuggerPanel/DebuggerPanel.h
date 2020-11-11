#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_DebuggerPanel.h"

namespace Studio
{
	class DebuggerPanel : public QDockWidget, public Ui_DebuggerPanel
	{
		Q_OBJECT

	public:
		DebuggerPanel(QWidget* parent = 0);
		~DebuggerPanel();

	protected:
		// ¹Ø±Õ
		//void closeEvent(QCloseEvent *event);
	};
}