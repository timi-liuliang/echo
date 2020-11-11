#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_LogPanel.h"
#include <engine/core/log/LogOutput.h>

namespace Studio
{
	//----------------------------------------
	// LogPanel 2010-12-30
	//----------------------------------------
	class LogPanel : public QDockWidget, public Echo::LogOutput, public Ui_LogPanel
	{
		Q_OBJECT

	public:
		LogPanel(QWidget* parent = 0);
		~LogPanel();

		// Out
		void OutMsg( int level,const char* msg, const char* icon);

	public:
		// log message
		virtual void logMessage(Level level, const Echo::String &msg) override;
		 
		// set path
		virtual void setPath(const Echo::String&){}

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