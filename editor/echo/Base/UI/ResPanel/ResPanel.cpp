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
#include <engine/core/render/interface/Material.h>

namespace Studio
{
	static ResPanel* g_inst = nullptr;

	// 构造函数
	ResPanel::ResPanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_resMenu(nullptr)
		, m_menuEditItem(nullptr)
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
		QObject::connect(m_previewHelper, SIGNAL(renamedRes(const QString, const QString)), this, SLOT(onRenamedRes(const QString, const QString)));
		QObject::connect(m_listView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_actionShowInExplorer, SIGNAL(triggered()), this, SLOT(showInExporer()));
		QObject::connect(m_actionNewFolder, SIGNAL(triggered()), this, SLOT(newFolder()));
		QObject::connect(m_actionRenameRes, SIGNAL(triggered()), this, SLOT(onRenameRes()));
		QObject::connect(m_actionDeleteRes, SIGNAL(triggered()), this, SLOT(onDeleteRes()));
		QObject::connect(m_actionDuplicateRes, SIGNAL(triggered()), this, SLOT(onDuplicateRes()));

		g_inst = this;
	}

	// 析构函数
	ResPanel::~ResPanel()
	{

	}

	// instance
	ResPanel* ResPanel::instance()
	{
		return g_inst;
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
		// refresh current dir
		m_dirModel->Clean();
		m_dirModel->Refresh();

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
				NodeTreePanel::instance()->onSelectRes(resPath);
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
				// edit res
				NodeTreePanel::instance()->onSelectRes(resPath);

				Echo::String ext = Echo::PathUtil::GetFileExt(resPath, true);
				if (ext == ".scene")
				{
					MainWindow::instance()->openNodeTree(resPath);
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
		QStandardItem* item = m_previewHelper->itemAt( point);

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

			QObject::connect(createResAction, SIGNAL(triggered()), this, SLOT(onCreateRes()));
		}
		m_resMenu->addMenu(createResMenu);
		if (item)
		{
			m_resMenu->addAction(m_actionDuplicateRes);
			m_resMenu->addAction(m_actionDeleteRes);
			m_resMenu->addAction(m_actionRenameRes);

			m_menuEditItem = item;
		}

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

	// new folder
	void ResPanel::newFolder()
	{
		Echo::String currentDir = m_currentDir;
		for (int i = 0; i < 65535; i++)
		{
			Echo::String newFolder = m_currentDir + (i!=0 ? Echo::StringUtil::Format("New Folder %d", i): "New Folder") + "/";
			if (!Echo::PathUtil::IsDirExist(newFolder))
			{
				Echo::PathUtil::CreateDir(newFolder);
				reslectCurrentDir();
				return;
			}
		}
	}

	// get unique file name
	bool ResPanel::getUniqueNewResSavePath( Echo::String& outNewPath, const Echo::String& className, const Echo::String& currentDir)
	{
		const Echo::Res::ResFun* resInfo = Echo::Res::getResFunByClassName(className);
		if (resInfo)
		{
			const Echo::String& extension = resInfo->m_ext;
			for (int i = 0; i < 65535; i++)
			{
				Echo::String newPath = Echo::StringUtil::Format("%sNew%s_%d%s", currentDir.c_str(), className.c_str(), i, extension.c_str());
				if (!Echo::PathUtil::IsFileExist(newPath))
				{
					if(Echo::IO::instance()->covertFullPathToResPath( newPath,outNewPath))
						return true;
				}
			}
		}

		return false;
	}

	// new res
	void ResPanel::onCreateRes()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String className = action->text().toStdString().c_str();
			Echo::ResPtr res = Echo::Res::createByClassName(className);
			if (res)
			{			
				Echo::String newSavePath;
				if (getUniqueNewResSavePath(newSavePath, className, m_currentDir))
				{
					res->setPath( newSavePath);
					res->save();

					reslectCurrentDir();
				}
			}
		}
	}

	// rename res
	void ResPanel::onRenameRes()
	{
		if(m_menuEditItem)
			m_previewHelper->editItem(m_menuEditItem);
	}

	// delete res
	void ResPanel::onDeleteRes()
	{
		if (m_menuEditItem)
		{
			Echo::String path = m_menuEditItem->data(Qt::UserRole).toString().toStdString().c_str();
			Echo::PathUtil::DelPath(path);

			reslectCurrentDir();
		}
	}

	// duplicate res
	void ResPanel::onDuplicateRes()
	{
		if (m_menuEditItem)
		{
			Echo::String fullPathName = m_menuEditItem->data(Qt::UserRole).toString().toStdString().c_str();
			if (!Echo::PathUtil::IsDir(fullPathName))
			{
				Echo::String path = Echo::PathUtil::GetFileDirPath(fullPathName);
				Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPathName, false);
				Echo::String fileExt = Echo::PathUtil::GetFileExt(fullPathName, true);
				for (int i = 0; i < 65535; i++)
				{
					Echo::String newPath = Echo::StringUtil::Format("%s%sCopy_%d%s", path.c_str(), fileName.c_str(), i, fileExt.c_str());
					if (!Echo::PathUtil::IsFileExist(newPath))
					{
						Echo::PathUtil::CopyFilePath(fullPathName, newPath, false);
						break;
					}
				}
			}
			else
			{
				Echo::String parentPath = Echo::PathUtil::GetParentPath(fullPathName);
				Echo::String lastPath = Echo::PathUtil::GetLastDirName(fullPathName);
				for (int i = 0; i < 65535; i++)
				{
					Echo::String newDir = Echo::StringUtil::Format("%s%sCopy_%d/", parentPath.c_str(), lastPath.c_str(), i);
					if (!Echo::PathUtil::IsDirExist(newDir))
					{
						Echo::PathUtil::CopyDir(fullPathName, newDir, false);
						break;
					}
				}
			}


			reslectCurrentDir();
		}
	}

	// on renamed res
	void ResPanel::onRenamedRes(const QString src, const QString dest)
	{
		// refresh current dir
		m_dirModel->Clean();
		m_dirModel->Refresh();
	}
}