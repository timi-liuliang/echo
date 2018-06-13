#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include <QDesktopServices>
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "MainWindow.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"
#include <engine/core/io/IO.h>
#include <engine/core/render/Material.h>

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

		QObject::connect(m_previewHelper, SIGNAL(clickedRes(const char*)), this, SLOT(onClickedPreviewRes(const char*)));
		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickedPreviewRes(const char*)));
		QObject::connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_actionShowInExplorer, SIGNAL(triggered()), this, SLOT(showInExporer()));
		//QObject::connect(m_actionNewMaterial, SIGNAL(triggered()), this, SLOT(newMaterial()));
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

	// click res
	void ResPanel::onClickedPreviewRes(const char* res)
	{
		if (!Echo::PathUtil::IsDir(res))
		{
			Echo::String resPath;
			if (Echo::IO::instance()->covertFullPathToResPath(res, resPath))
			{
				NodeTreePanel::instance()->showResProperty(resPath);
			}
		}
	}

	// double click res
	void ResPanel::onDoubleClickedPreviewRes(const char* res)
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
				else if (ext == ".material")
				{
					NodeTreePanel::instance()->showResProperty( resPath);
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

		// create res
		QMenu* createResMenu = new QMenu("New");
		createResMenu->addAction(m_actionNewFolder);
		createResMenu->addSeparator();

		Echo::StringArray allRes;
		Echo::Class::getChildClasses(allRes, "Res", true);
		for (const Echo::String& res : allRes)
		{
			QAction* createResAction = new QAction(this);
			createResAction->setText(res.c_str());
			createResMenu->addAction(createResAction);
		}
		m_resMenu->addMenu(createResMenu);

		m_resMenu->addSeparator();
		m_resMenu->addAction(m_actionShowInExplorer);

		m_resMenu->exec(QCursor::pos());
	}

	// show current dir
	void ResPanel::showInExporer()
	{
		QString openDir = m_currentDir.c_str();
		if (!openDir.isEmpty())
		{
			QDesktopServices::openUrl(openDir);
		}		
	}

	// new material
	void ResPanel::newMaterial()
	{
		for (int i = 0; i < Echo::Math::MAX_I16; i++)
		{
			Echo::String fileName = Echo::StringUtil::Format("NewMaterial_%d.material", i);
			if (!Echo::PathUtil::IsFileExist(m_currentDir + fileName))
			{
				Echo::String resPath;
				if (Echo::IO::instance()->covertFullPathToResPath(m_currentDir, resPath))
				{
					Echo::String savePath = resPath + fileName;
					Echo::MaterialRef material = Echo::Material::create();
					material->setPath(savePath);
					material->save();

					reslectCurrentDir();
				}

				break;
			}
		}
	}
}