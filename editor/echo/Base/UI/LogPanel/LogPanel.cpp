#include "LogPanel.h"
#include <QListWidgetItem>

namespace Studio
{
	// 构造函数
	LogPanel::LogPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
	{
		setupUi(this);

		QObject::connect(this, SIGNAL(postMessage(int, QString)),this, SLOT(onLogMessage(int, QString)), Qt::QueuedConnection);
		QObject::connect(m_clear, SIGNAL(clicked()), this, SLOT(onClearMessage()));
	}

	// 析构函数
	LogPanel::~LogPanel()
	{
	}

	// 获取目标名称
	const Echo::String& LogPanel::getName() const 
	{ 
		static Echo::String logPanel = "LogPanel";

		return logPanel; 
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
		case LL_WARNING: Warning(msg.toStdString().c_str()); break;
		case LL_ERROR:
		case LL_FATAL:	Error(msg.toStdString().c_str());	break;
		default:								break;
		}
	}

	// 警告
	void LogPanel::Warning( const char* msg)
	{
		QListWidgetItem* warningItem = new QListWidgetItem( QString::fromLocal8Bit(msg));
		if( warningItem)
		{	
			warningItem->setIcon( QIcon(":/icon/Icon/CheckWarning.png"));
			m_logList->addItem( warningItem);
		}

		// 显示界面
		setVisible( true);
	}

	// 错误
	void LogPanel::Error( const char* msg)
	{
		QListWidgetItem* errorItem = new QListWidgetItem( QString::fromLocal8Bit(msg));
		if( errorItem)
		{	
			errorItem->setIcon( QIcon(":/icon/Icon/CheckError.png"));
			m_logList->addItem( errorItem);
		}

		// 显示界面
		setVisible( true);
	}

	void LogPanel::onClearMessage()
	{
		m_logList->clear();
	}

	// 关闭
	//void LogPanel::closeEvent(QCloseEvent *event)
	//{
	//	m_logList->clear();

	//	QDialog::closeEvent( event);
	//}
}