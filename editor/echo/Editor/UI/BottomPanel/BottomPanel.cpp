#include "BottomPanel.h"
#include <QListWidgetItem>
#include <QFile>
#include <QUiLoader>
#include "DebuggerPanel.h"
#include "LogPanel.h"
#include <engine/core/editor/qt/QUiLoader.h>
#include "Document.h"
#include "Studio.h"

namespace Studio
{
	BottomPanel::BottomPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
	{
		setupUi(this);

		m_documentPanel = EchoNew(DocumentPanel(this));
		m_debuggerPanel = EchoNew(DebuggerPanel(this));

		m_tabWidget->clear();
		m_tabWidget->addTab(AStudio::instance()->getLogPanel(), "Log");
		m_tabWidget->addTab(m_debuggerPanel, "Debugger");
		m_tabWidget->addTab(m_documentPanel, "Document");

		onTabIdxChanged(0);

		QObject::connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabIdxChanged(int)));
	}

	BottomPanel::~BottomPanel()
	{
	}

	QWidget* BottomPanel::getTab(const Echo::String& tabName)
	{
		for (int i = 0; i < m_tabWidget->count(); i++)
		{
			if (m_tabWidget->tabText(i) == tabName.c_str())
			{
				return m_tabWidget->widget(i);
			}
		}

		return nullptr;
	}

	void BottomPanel::setTabVisible(const Echo::String& tabName, bool isVisible)
	{
		QWidget* page = getTab(tabName.c_str());
		if (page)
		{
			page->setVisible(true);
			m_tabWidget->setCurrentWidget(page);
		}
	}

	bool BottomPanel::isTabVisible(const Echo::String& tabName)
	{
		QWidget* page = getTab(tabName.c_str());
		return page ? page->isVisible() : false;
	}

	void BottomPanel::onTabIdxChanged(int idx)
	{
		QWidget* widget = m_tabWidget->currentWidget();
		setWindowTitle(widget->windowTitle());
	}

	void BottomPanel::showBottomPanel(Echo::BottomPanelTab* bottomPanel)
	{
		// display
		if (bottomPanel->getUiPtr())
		{
			Echo::String title = bottomPanel->getUiPtr()->windowTitle().toStdString().c_str();
			QWidget* page = getTab(title);
			if (!page)
			{
				QWidget* widget = bottomPanel->getUiPtr();
				m_tabWidget->addTab( widget, title.c_str());
			}

			setTabVisible(title, true);
		}	
	}
}