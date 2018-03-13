#pragma once

#include <QDockWidget>
#include <QString>
#include "ui_LogPanel.h"
#include <Engine/Core.h>
#include <engine/core/Util/Log.h>

namespace Studio
{
	//----------------------------------------
	// 输出 主窗口 2010-12-30
	//----------------------------------------
	class LogPanel : public QDockWidget, public Echo::Log, public Ui_LogPanel
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
		// 设置日志名称
		virtual void setLogName(const Echo::String &logName) {}

		// 获取目标名称
		virtual const Echo::String& getName() const;

		// 设置日志过滤级别
		virtual void setLogLevel(LogLevel level) {}

		// 日志输出
		virtual void logMessage(LogLevel level, const Echo::String &msg);
		 
		// 获取日志级别
		virtual LogLevel getLogLevel() const { return LL_WARNING;}

		virtual void setPath(const Echo::String&) {}

	signals:
		void postMessage(int level, QString);

	private slots:
		void onLogMessage(int level, QString);
		void onClearMessage();


	private:
		Echo::ui32			m_sameMessageNum;
		Echo::String		m_lastMessage;
		int			m_lastLevel;
	};
}