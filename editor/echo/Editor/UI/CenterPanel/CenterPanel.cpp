#include "CenterPanel.h"
#include <QListWidgetItem>
#include <QFile>
#include <QUiLoader>
#include "DebuggerPanel.h"
#include "LogPanel.h"
#include <engine/core/editor/qt/QUiLoader.h>
#include "Document.h"
#include "Studio.h"
#include "MainWindow.h"

namespace Studio
{
	class CenterPanelCustomSizeHint : public QWidget
	{
	public:
		CenterPanelCustomSizeHint(QWidget* parent, float widthRatio=0.75f, float heightRation=0.65f)
			: QWidget(parent), m_widthRatio(widthRatio), m_heightRation(heightRation)
		{}

		// size hint
		virtual QSize sizeHint() const
		{
			float width = MainWindow::instance()->width() * m_widthRatio;
			float height = MainWindow::instance()->height() * m_heightRation;
			return QSize(width, height);
		}

	private:
		float	m_widthRatio = 0.75f;
		float	m_heightRation = 0.65f;
	};

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

	void CenterPanel::showPanel(Echo::PanelTab* panel, float widthRatio, float heightRation)
	{
		// display
		if (panel->getUiPtr())
		{
			Echo::String title = panel->getUiPtr()->windowTitle().toStdString().c_str();
			QWidget* page = getTab(title);
			if (!page)
			{
				QWidget* widget = panel->getUiPtr();
				if (widget)
				{
					QWidget* sizeHintWidget = new CenterPanelCustomSizeHint(m_tabWidget, widthRatio, heightRation);
					QHBoxLayout* horizonLayout = new QHBoxLayout(sizeHintWidget);
					horizonLayout->setSpacing(0);
					horizonLayout->setContentsMargins(0, 0, 0, 0);
					horizonLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
					horizonLayout->addWidget(widget);
					m_tabWidget->addTab(sizeHintWidget, title.c_str());
				}
			}

			setTabVisible(title, true);
			setVisible(true);
		}	
	}
}
