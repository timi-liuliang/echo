#include "BottomPanel.h"
#include <QListWidgetItem>
#include "TimelinePanel.h"
#include "DebuggerPanel.h"
#include "LogPanel.h"
#include "Document.h"
#include "Studio.h"

namespace Studio
{
	// 构造函数
	BottomPanel::BottomPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
	{
		setupUi(this);

		m_documentPanel = EchoNew(DocumentPanel(this));
		m_timelinePanel = EchoNew(TimelinePanel(this));
		m_debuggerPanel = EchoNew(DebuggerPanel(this));

		m_tabWidget->clear();
		m_tabWidget->addTab(AStudio::instance()->getLogPanel(), "Log");
		m_tabWidget->addTab(m_debuggerPanel, "Debugger");
		m_tabWidget->addTab(m_timelinePanel, "TimeLine");
		m_tabWidget->addTab(m_documentPanel, "Document");

		//setTabVisible("DocumentPanel", false);

		onTabIdxChanged(0);

		QObject::connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabIdxChanged(int)));
	}

	// 析构函数
	BottomPanel::~BottomPanel()
	{
	}

	// show tab
	void BottomPanel::setTabVisible(const Echo::String& tabName, bool isVisible)
	{
		QWidget* page = m_tabWidget->findChild<QWidget*>(tabName.c_str());
		if (page)
		{
			page->setVisible(true);
			m_tabWidget->setCurrentWidget(page);
		}
	}

	bool BottomPanel::isTabVisible(const Echo::String& tabName)
	{
		QWidget* page = m_tabWidget->findChild<QWidget*>(tabName.c_str());
		return page ? page->isVisible() : false;
	}

	// on tab index changed
	void BottomPanel::onTabIdxChanged(int idx)
	{
		QWidget* widget = m_tabWidget->currentWidget();
		setWindowTitle(widget->windowTitle());
	}
}