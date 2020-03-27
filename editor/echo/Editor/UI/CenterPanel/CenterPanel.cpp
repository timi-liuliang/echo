#include "CenterPanel.h"
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
	CenterPanel::CenterPanel(QWidget* parent/* = 0*/)
		: QDockWidget( parent)
	{
		setupUi(this);

		m_tabWidget->clear();

		QObject::connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(onTabIdxChanged(int)));

		setVisible(false);
	}

	CenterPanel::~CenterPanel()
	{
        m_tabWidget->disconnect();
	}

	QWidget* CenterPanel::getTab(const Echo::String& tabName)
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

	void CenterPanel::setTabVisible(const Echo::String& tabName, bool isVisible)
	{
		QWidget* page = getTab(tabName.c_str());
		if (page)
		{
			page->setVisible(true);
			m_tabWidget->setCurrentWidget(page);
		}
	}

	bool CenterPanel::isTabVisible(const Echo::String& tabName)
	{
		QWidget* page = getTab(tabName.c_str());
		return page ? page->isVisible() : false;
	}

	void CenterPanel::onTabIdxChanged(int idx)
	{
		QWidget* widget = m_tabWidget->currentWidget();
		setWindowTitle(widget->windowTitle());
	}

	void CenterPanel::showPanel(Echo::PanelTab* bottomPanel)
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
			setVisible(true);
		}	
	}
}
