#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Root.h"

namespace Studio
{
	// 构造函数
	ResPanel::ResPanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
	{
		setupUi( this);

		// 目录树型结构
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
		m_dirModel->Clean();

		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));

		m_previewHelper = new QT_UI::QPreviewHelper(m_listView);

		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickPreviewRes(const char*)));
	}

	// 析构函数
	ResPanel::~ResPanel()
	{

	}

	// call when open project
	void ResPanel::onOpenProject()
	{
		m_dirModel->clear();

		QStringList titleLable;
		titleLable << "Res://";
		m_dirModel->setHorizontalHeaderLabels(titleLable);

		m_dirModel->SetRootPath(Echo::Root::instance()->getResPath().c_str(), "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		onSelectDir(Echo::Root::instance()->getResPath().c_str());

		resizeEvent(nullptr);
	}

	// 选择文件夹
	void ResPanel::onSelectDir(const char* dir)
	{
		m_previewHelper->clear();
		m_previewHelper->setPath(dir, nullptr);
	}

	// double click res
	void ResPanel::onDoubleClickPreviewRes(const char* res)
	{
		Echo::String resPath;
		if (Echo::IO::instance()->covertFullPathToResPath(res, resPath))
		{
			Echo::Node* node = Echo::Node::load(resPath);
			if (node)
			{
				Echo::Node* old = Studio::EchoEngine::Instance()->getCurrentEditNode();
				if (old)
				{
					old->queueFree();
				}

				Studio::EchoEngine::Instance()->setCurrentEditNode(node);

				NodeTreePanel::instance()->refreshNodeTreeDisplay();
			}
		}
	}

	// reimplement reiszeEvent function
	void ResPanel::resizeEvent(QResizeEvent * e)
	{
		m_previewHelper->onListViewResize();
	}
}