#include "LogPanel.h"
#include <QListWidgetItem>

namespace Studio
{
	// 构造函数
	LogPanel::LogPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
		, Log("LogPanel")
		, m_sameMessageNum(0)
		, m_lastLevel(LL_INVALID)
	{
		setupUi(this);

		QObject::connect(this, SIGNAL(postMessage(int, QString)),this, SLOT(onLogMessage(int, QString)), Qt::QueuedConnection);
		QObject::connect(m_clear, SIGNAL(clicked()), this, SLOT(onClearMessage()));
	}

	// 析构函数
	LogPanel::~LogPanel()
	{
	}

	// 日志输出
	void LogPanel::logMessage(LogLevel level, const Echo::String &msg)
	{
		emit postMessage(level, QString::fromStdString(msg));
	}

	void LogPanel::onLogMessage(int level, QString msg)
	{
		switch (level)
		{
		case LL_WARNING: OutMsg(level, msg.toStdString().c_str(), ":/icon/Icon/log/CheckWarning.png"); break;
		case LL_ERROR:
		case LL_FATAL:	OutMsg(level, msg.toStdString().c_str(), ":/icon/Icon/log/CheckError.png");	break;
		case LL_INFO:
		case LL_DEBUG:	OutMsg(level, msg.toStdString().c_str(), ":/icon/Icon/log/info.png"); break;
		default:								break;
		}
	}

	// 输出消息
	void LogPanel::OutMsg( int level, const char* msg, const char* icon)
	{
		if (msg == m_lastMessage && level==m_lastLevel)
		{
			m_sameMessageNum++;
			QListWidgetItem* lastItem = m_logList->item(m_logList->count()-1);
			lastItem->setText(QString::fromLocal8Bit(Echo::StringUtil::Format("(%d) %s", m_sameMessageNum, msg).c_str()));
		}
		else
		{
			m_sameMessageNum = 1;
			QListWidgetItem* warningItem = new QListWidgetItem(QString::fromLocal8Bit(msg));
			if (warningItem)
			{
				warningItem->setIcon(QIcon(icon));
				m_logList->addItem(warningItem);
			}
		}

		m_lastMessage = msg;
		m_lastLevel = level;

		// 显示界面
		setVisible( true);

		m_logList->scrollToBottom();
	}

	void LogPanel::onClearMessage()
	{
		m_logList->clear();
		m_sameMessageNum = 0;
		m_lastLevel = -1;
		m_lastMessage = "";
	}
}