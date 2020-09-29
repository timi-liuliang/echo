#include <QtGui>
#include <QDateTime>
#include <QMenuBar>
#include <QDesktopServices>
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include "EchoEngine.h"
#include "MainWindow.h"
#include "Studio.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/main/Engine.h"
#include <engine/core/io/IO.h>
#include <engine/core/resource/Res.h>
#include <engine/core/render/base/material.h>
#include <engine/core/editor/importer.h>

namespace Studio
{
	static ResPanel* g_inst = nullptr;

	ResPanel::ResPanel( QWidget* parent/*=0*/)
		: QDockWidget( parent)
		, m_resMenu(nullptr)
		, m_menuEditItem(nullptr)
		, m_viewTypeGrid(true)
	{
		setupUi( this);

		// directory tree structre
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
        m_resDirView->setAttribute(Qt::WA_MacShowFocusRect,0);
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
		QObject::connect(m_actionCopyPath, SIGNAL(triggered()), this, SLOT(onCopyResPath()));
		QObject::connect(m_viewTypeButton, SIGNAL(clicked()), this, SLOT(onSwitchResVeiwType()));
		QObject::connect(m_searchLineEdit, SIGNAL(textChanged(const QString &)), this, SLOT(onSearchTextChanged()));

		g_inst = this;
	}

	ResPanel::~ResPanel()
	{

	}

	ResPanel* ResPanel::instance()
	{
		return g_inst;
	}

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

	void ResPanel::onSelectDir(const char* dir)
	{
		m_currentDir = dir;

		m_previewHelper->clear();

		bool isIncludePreDir = dir == Echo::Engine::instance()->getResPath() ? false : true;
		m_previewHelper->setPath(dir, nullptr, isIncludePreDir);

		// watch current dir
		EchoSafeDelete(m_filesystemWatcher, QFileSystemWatcher);
		m_filesystemWatcher = EchoNew(QFileSystemWatcher);
		m_filesystemWatcher->addPath(m_currentDir.c_str());
		QObject::connect(m_filesystemWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(onWatchFileChanged(const QString&)));
		QObject::connect(m_filesystemWatcher, SIGNAL(directoryChanged(const QString&)), this, SLOT(onWatchFileDirChanged(const QString&)));
	}

	void ResPanel::reslectCurrentDir()
	{
		// refresh current dir
		m_dirModel->Clean();
		m_dirModel->Refresh();

		if (!m_currentDir.empty())
			onSelectDir(m_currentDir.c_str());
	}

	void ResPanel::onWatchFileChanged(const QString& file)
	{

	}

	void ResPanel::onWatchFileDirChanged(const QString& dir)
	{
		if (m_currentDir == dir.toStdString().c_str())
		{
			reslectCurrentDir();
		}
	}

	void ResPanel::onClickedPreviewRes(const char* res)
	{
		if (!Echo::PathUtil::IsDir(res))
		{
			Echo::String resPath;
			if (Echo::IO::instance()->convertFullPathToResPath(res, resPath))
			{
				NodeTreePanel::instance()->onSelectRes(resPath);
			}
		}
	}

	void ResPanel::onDoubleClickedPreviewRes(const char* resFullPath)
	{
		if (Echo::PathUtil::IsDir(resFullPath))
		{
			m_dirModel->setCurrentSelect(resFullPath);
		}
		else
		{
			Echo::ResPtr res = Echo::Res::createByFileExtension(Echo::PathUtil::GetFileExt(resFullPath, true), true);
			if (res && res->isPackage())
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(resFullPath, resPath))
				{
					Echo::ResPtr selectRes = Echo::Res::get(resPath);
					m_previewHelper->clear();
					m_previewHelper->setRes(selectRes, nullptr, true);
				}
			}
			else
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(resFullPath, resPath))
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
					else if (ext == ".shader")
					{
						MainWindow::instance()->openShaderEditor(resPath);
					}
				}
			}
		}

	}

	void ResPanel::resizeEvent(QResizeEvent * e)
	{
		m_previewHelper->onListViewResize();
	}

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
			// res may don't have create method, which means this type of resource
			// can't be create by echo
			const Echo::Res::ResFun* resFun = Echo::Res::getResFunByClassName(res);
			if (resFun->m_cfun)
			{
				QAction* createResAction = new QAction(this);
				createResAction->setText(res.c_str());
				createResMenu->addAction(createResAction);

				QObject::connect(createResAction, SIGNAL(triggered()), this, SLOT(onCreateRes()));
			}
		}
		m_resMenu->addMenu(createResMenu);
		if (item)
		{
			m_resMenu->addAction(m_actionDuplicateRes);
			m_resMenu->addAction(m_actionDeleteRes);
			m_resMenu->addAction(m_actionRenameRes);

			m_resMenu->addSeparator();
			m_resMenu->addAction(m_actionCopyPath);

			m_menuEditItem = item;
		}
		else
		{
			m_resMenu->addSeparator();

			// import res
			QMenu* importResMenu = new QMenu("Import");

			Echo::StringArray allImporter;
			Echo::Class::getChildClasses(allImporter, "Importer", true);
			for (const Echo::String& importer : allImporter)
			{
				Echo::Importer* importerObj = (Echo::Importer*)Echo::Class::create(importer.c_str());
				if (importerObj)
				{
					QAction* importResAction = new QAction(this);
					importResAction->setText(importerObj->getName());
					importResAction->setToolTip(importer.c_str());
					importResMenu->addAction(importResAction);

					QObject::connect(importResAction, SIGNAL(triggered()), this, SLOT(onImportRes()));

					EchoSafeDelete(importerObj, Importer);
				}
			}

			m_resMenu->addMenu(importResMenu);
		}

		m_resMenu->addSeparator();
		m_resMenu->addAction(m_actionShowInExplorer);

		m_resMenu->exec(QCursor::pos());
	}

	void ResPanel::showInExporer()
	{
		QString openDir = m_currentDir.c_str();
		if (!openDir.isEmpty())
		{
#ifdef ECHO_PLATFORM_WINDOWS
			QDesktopServices::openUrl(openDir);
#else
            QDesktopServices::openUrl(QUrl("file://" + openDir));
#endif
		}		
	}

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
					if(Echo::IO::instance()->convertFullPathToResPath( newPath,outNewPath))
						return true;
				}
			}
		}

		return false;
	}

	void ResPanel::onCreateRes()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String className = action->text().toStdString().c_str();
			Echo::ResPtr res = (Echo::Res*)Echo::Class::create(className);
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

	void ResPanel::onImportRes()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String className = action->toolTip().toStdString().c_str();
			Echo::Importer* importer = (Echo::Importer*)Echo::Class::create(className);
			if (importer)
			{
				importer->run(m_currentDir.c_str());
			}
		}
	}

	void ResPanel::onRenameRes()
	{
		if(m_menuEditItem)
			m_previewHelper->editItem(m_menuEditItem);
	}

	void ResPanel::onDeleteRes()
	{
		if (m_menuEditItem)
		{
			Echo::String path = m_menuEditItem->data(Qt::UserRole).toString().toStdString().c_str();
			Echo::PathUtil::DelPath(path);

			reslectCurrentDir();
		}
	}

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

	void ResPanel::onCopyResPath()
	{
		if (m_menuEditItem)
		{
			Echo::String path = m_menuEditItem->data(Qt::UserRole).toString().toStdString().c_str();
			if (Echo::IO::instance()->convertFullPathToResPath(path, path))
			{
				QClipboard* clipboard = QApplication::clipboard();
				clipboard->setText(path.c_str());
			}
		}
	}

	void ResPanel::onRenamedRes(const QString src, const QString dest)
	{
		// refresh current dir
		m_dirModel->Clean();
		m_dirModel->Refresh();
	}

	void ResPanel::onSwitchResVeiwType()
	{
		m_viewTypeGrid = !m_viewTypeGrid;
		if (m_viewTypeGrid)
		{
			m_previewHelper->setUseIconMode();

			m_viewTypeButton->setIcon(QIcon(":/icon/Icon/res/view_type_list.png"));
			m_viewTypeButton->setToolTip("List");
		}
		else
		{
			m_previewHelper->setUseListMode();

			m_viewTypeButton->setIcon(QIcon(":/icon/Icon/res/view_type_grid.png"));
			m_viewTypeButton->setToolTip("Thumbnail");
		}

		// save config
		AStudio::instance()->getConfigMgr()->setValue("ResPanel_ResViewType", m_viewTypeGrid ? "Grid" : "List");
	}

	void ResPanel::onSearchTextChanged()
	{
		Echo::String pattern = m_searchLineEdit->text().toStdString().c_str();
		m_previewHelper->setFilterPattern( pattern.c_str());
	}

	void ResPanel::recoverEditSettings()
	{
		Echo::String viewType = AStudio::instance()->getConfigMgr()->getValue("ResPanel_ResViewType");
		if (!viewType.empty())
		{
			if (m_viewTypeGrid && viewType != "Grid")
			{
				onSwitchResVeiwType();
			}
		}
	}
}
