#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "MainWindow.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"
#include <engine/core/io/IO.h>

namespace Studio
{
	// 构造函数
	ResPanel::ResPanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_resMenu(nullptr)
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
		QObject::connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_actionNewShader, SIGNAL(triggered()), this, SLOT(newShader()));
		QObject::connect(m_actionNewMaterial, SIGNAL(triggered()), this, SLOT(newMaterial()));
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

		m_dirModel->SetRootPath(Echo::Engine::instance()->getResPath().c_str(), "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		onSelectDir(Echo::Engine::instance()->getResPath().c_str());

		resizeEvent(nullptr);
	}

	// 选择文件夹
	void ResPanel::onSelectDir(const char* dir)
	{
		m_currentDir = dir;

		m_previewHelper->clear();

		bool isIncludePreDir = dir == Echo::Engine::instance()->getResPath() ? false : true;
		m_previewHelper->setPath(dir, nullptr, isIncludePreDir);
	}

	// 重新选择当前文件夹
	void ResPanel::reslectCurrentDir()
	{
		if (!m_currentDir.empty())
			onSelectDir(m_currentDir.c_str());
	}

	// double click res
	void ResPanel::onDoubleClickPreviewRes(const char* res)
	{
		if (Echo::PathUtil::IsDir(res))
		{
			m_dirModel->setCurrentSelect(res);
		}
		else
		{
			Echo::String resPath;
			if (Echo::IO::instance()->covertFullPathToResPath(res, resPath))
			{
				Echo::String ext = Echo::PathUtil::GetFileExt(resPath, true);
				if (ext == ".scene")
				{
					Echo::Node* node = Echo::Node::load(resPath);
					if (node)
					{
						Echo::Node* old = Studio::EchoEngine::instance()->getCurrentEditNode();
						if (old)
						{
							old->queueFree();
						}

						Studio::EchoEngine::instance()->setCurrentEditNode(node);
						Studio::EchoEngine::instance()->setCurrentEditNodeSavePath(res);

						NodeTreePanel::instance()->refreshNodeTreeDisplay();
					}
				}
				else if (ext == ".lua")
				{
					MainWindow::instance()->openLuaScript(resPath);
				}
			}
		}
	}

	// reimplement reiszeEvent function
	void ResPanel::resizeEvent(QResizeEvent * e)
	{
		m_previewHelper->onListViewResize();
	}

	// node tree widget show menu
	void ResPanel::showMenu(const QPoint& point)
	{
		EchoSafeDelete(m_resMenu, QMenu);
		m_resMenu = EchoNew(QMenu);
		m_resMenu->addAction(m_actionShowInExplorer);
		m_resMenu->addSeparator();
		m_resMenu->addAction(m_actionNewFolder);
		m_resMenu->addSeparator();
		m_resMenu->addAction(m_actionNewShader);
		m_resMenu->addAction(m_actionNewMaterial);
		m_resMenu->exec(QCursor::pos());
	}

	// new shader
	void ResPanel::newShader()
	{

	}

	// new material
	void ResPanel::newMaterial()
	{

	}
}