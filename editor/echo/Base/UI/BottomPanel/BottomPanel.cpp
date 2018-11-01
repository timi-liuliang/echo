#include "BottomPanel.h"
#include <QListWidgetItem>
#include <QFile>
#include <QUiLoader>
#include "DebuggerPanel.h"
#include "LogPanel.h"
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
		// create widget by ui file
		if (!bottomPanel->getUiPtr() && bottomPanel->getUiFile())
		{
			Echo::String path = AStudio::instance()->getRootPath() + Echo::String(bottomPanel->getUiFile());

			QFile file(path.c_str());
			file.open(QFile::ReadOnly);

			QUiLoader loader;
			QWidget* widget = loader.load(&file, nullptr);

			bottomPanel->setUiPtr(widget);

			file.close();
		}

		// display
		if (bottomPanel->getUiPtr())
		{
			QWidget* page = getTab(bottomPanel->getTitle());
			if (!page)
			{
				QWidget* widget = (QWidget*)bottomPanel->getUiPtr();
				m_tabWidget->addTab( widget, bottomPanel->getTitle());
			}

			setTabVisible(bottomPanel->getTitle(), true);
		}	
	}
}