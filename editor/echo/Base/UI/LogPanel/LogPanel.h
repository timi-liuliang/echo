#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_LogPanel.h"
#include <engine/core/log/Log.h>

namespace Studio
{
	//----------------------------------------
	// 输出 主窗口 2010-12-30
	//----------------------------------------
	class LogPanel : public QWidget, public Echo::Log, public Ui_LogPanel
	{
		Q_OBJECT

	public:
		// 构造函数
		LogPanel(QWidget* parent = 0);

		// 析构函数
		~LogPanel();

		// 输出消息
		void OutMsg( int level,const char* msg, const char* icon);

	public:
		// 日志输出
		virtual void logMessage(LogLevel level, const Echo::String &msg);
		 

		virtual void setPath(const Echo::String&) {}

	signals:
		void postMessage(int level, QString);

	private slots:
		void onLogMessage(int level, QString);
		void onClearMessage();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// copy log content
		void copyLogContent();

	private:
		Echo::ui32			m_sameMessageNum;
		Echo::String		m_lastMessage;
		int					m_lastLevel;
		QMenu*				m_menu;
		QListWidgetItem*	m_currentSelectItem;
	};
}