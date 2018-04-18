#pragma once

#include <QProcess>
#include "ui_MainWindow.h"

namespace Studio
{
	class ResPanel;
	class NodeTreePanel;
	class TimelinePanel;
	class DebuggerPanel;
	class MainWindow : public QMainWindow, public Ui_MainWindow
	{
		Q_OBJECT

	public:
		MainWindow(QMainWindow* parent=0);
		~MainWindow();

		// 打开项目时调用
		void onOpenProject();

		// 打开文件
		void OpenProject(const char* projectName);

	protected:
		void closeEvent(QCloseEvent *event);

	public slots:
		// 保存文件
		void onSaveProject();

		// game mode
		void onPlayGame();

	protected:
		ResPanel*		m_resPanel;
		NodeTreePanel*	m_scenePanel;
		TimelinePanel*	m_timelinePanel;
		DebuggerPanel*	m_debuggerPanel;
		QProcess		m_gameProcess;
	};
}