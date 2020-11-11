#include "LogPanel.h"
#include <QListWidgetItem>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <engine/core/util/StringUtil.h>

namespace Studio
{
	LogPanel::LogPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
		, LogOutput("LogPanel")
		, m_sameMessageNum(0)
		, m_lastLevel(LL_INVALID)
		, m_menu(nullptr)
		, m_currentSelectItem(nullptr)
	{
		setupUi(this);
        
        // hide mac focus rect
        m_logList->setAttribute(Qt::WA_MacShowFocusRect,0);

		QObject::connect(this, SIGNAL(postMessage(int, QString)),this, SLOT(onLogMessage(int, QString)), Qt::QueuedConnection);
		QObject::connect(m_clear, SIGNAL(clicked()), this, SLOT(onClearMessage()));
		QObject::connect(m_logList, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_actionCopyContent, SIGNAL(triggered()), this, SLOT(copyLogContent()));
	}

	LogPanel::~LogPanel()
	{
	}

	void LogPanel::logMessage(Level level, const Echo::String &msg)
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

		// show log panel
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

	// node tree widget show menu
	void LogPanel::showMenu(const QPoint& point)
	{
		m_currentSelectItem = m_logList->itemAt(point);
		if (m_currentSelectItem)
		{
			EchoSafeDelete(m_menu, QMenu);
			m_menu = EchoNew(QMenu);

			m_menu->addAction(m_actionCopyContent);

			m_menu->exec(QCursor::pos());
		}
	}

	// copy log content
	void LogPanel::copyLogContent()
	{
		if (m_currentSelectItem)
		{
			Echo::String text = m_currentSelectItem->text().toStdString().c_str();
			QClipboard* clipboard = QApplication::clipboard();
			clipboard->setText(text.c_str());
		}
	}
}
