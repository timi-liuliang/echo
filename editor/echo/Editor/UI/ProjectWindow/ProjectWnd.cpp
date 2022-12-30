#include <QMessageBox>
#include <QStandardItem>
#include <QDesktopServices>
#include <QPainter>
#include "ProjectWnd.h"
#include <qfiledialog.h>
#include "Studio.h"
#include "Update.h"
#include "MacHelper.h"
#include <engine/core/log/Log.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/game_settings.h>

namespace Studio
{
	ProjectWnd::ProjectWnd(QMainWindow* parent /* = 0 */)
		: QMainWindow(parent)
	{
		setupUi(this);

#ifdef ECHO_PLATFORM_WINDOWS
		// hide window hwnd
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
        // set title bar color
        setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
        //macChangeTitleBarColor( winId(), 0.0, 0.0, 0.0);
        m_menuBar->setNativeMenuBar(false);
        m_versionListWidget->setAttribute(Qt::WA_MacShowFocusRect,0);
        
        // adjust size
        // adjustWidgetSizeByOS(this);
#endif

		// set top left corner icon
		m_menuBar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		m_previewerWidget = new QT_UI::QPreviewWidget(m_recentProject);
		m_previewerWidget->setContextMenuPolicy(Qt::CustomContextMenu);
		m_previewerWidget->isNeedFullPath(true);

		m_layout->addWidget(m_previewerWidget);

		QObject::connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(openNewProject(int)));
		QObject::connect(m_previewerWidget, SIGNAL(Signal_onDoubleClickedItem(const QString&)), this, SLOT(onDoubleClicked(const QString&)));
		QObject::connect(m_previewerWidget, SIGNAL(Signal_onClickedItem(const QString&)), this, SLOT(onClicked(const QString&)));
        QObject::connect(m_previewerWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showMenu(const QPoint&)));
		QObject::connect(m_versionListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDownloadNewVersion(QListWidgetItem*)));
        QObject::connect(m_actionRemoveProject, SIGNAL(triggered()), this, SLOT(onRemoveProject()));
		QObject::connect(m_actionShowInExplorer, SIGNAL(triggered()), this, SLOT(onShowProjectInExplorer()));

		// show all updateable version echo
		showAllUpdateableVersion();
	}

	ProjectWnd::~ProjectWnd()
	{
		delete m_previewerWidget;
		m_previewerWidget = NULL;
	}

    void ProjectWnd::loadAllRecentProjects()
    {
        Echo::list<Echo::String>::type recentProjects;
        EditorConfig::instance()->getAllRecentProject(recentProjects);

        m_previewerWidget->clearAllItems();
        for ( Echo::String& project : recentProjects)
        {
            addRecentProject(project.c_str());
        }
    }

	void ProjectWnd::addRecentProject(const char* project)
	{
		if ( m_previewerWidget && NULL != project && 0 != project[0])
		{
			Echo::String icon = Echo::PathUtil::GetFileDirPath(project) + "Res/icon.png";
			icon = Echo::PathUtil::IsFileExist(icon) ? icon : ":/icon/Icon/error/delete.png";
			m_previewerWidget->addItem(project, icon.c_str());
		}
	}

	void ProjectWnd::openNewProject(int index)
	{
		if ( 2 == index )
		{
			Echo::String defaultDir = Echo::PathUtil::GetCurrentDir() + "/examples/";
			QString projectName = QFileDialog::getOpenFileName(this, tr("Open Project"), defaultDir.c_str(), tr("(*.echo)"));
			if ( !projectName.isEmpty())
			{
				openProject(projectName.toStdString().c_str());
			}
			else
			{
				tabWidget->setCurrentIndex(0);
			}
		}

		// create project
		else if (1 == index)
		{
			Echo::String newProjectPathName = newProject();
			if (!newProjectPathName.empty())
			{
				// 5.open project
				AStudio::instance()->getMainWindow()->showMaximized();
				AStudio::instance()->OpenProject(newProjectPathName.c_str());
				AStudio::instance()->getRenderWindow();

				close();
			}
			else
			{
				tabWidget->setCurrentIndex(0);
			}
		}
	}

    static void copyQtFile(const Echo::String& qtFile, const Echo::String& writePath)
    {
        QFile qfile(qtFile.c_str());
        if (qfile.open(QIODevice::ReadOnly))
        {
			Echo::String savePath = Echo::PathUtil::GetFileDirPath(writePath.c_str());
			if(!Echo::PathUtil::IsDirExist(savePath))
				Echo::PathUtil::CreateDir(savePath);

            // write files
            QFile writeFile(writePath.c_str());
            if (writeFile.open(QIODevice::WriteOnly))
            {
                writeFile.write(qfile.readAll());
                writeFile.close();
            }

            qfile.close();
        }
    }

	Echo::String ProjectWnd::newProject()
	{        
		QString projectName = QFileDialog::getSaveFileName(this, tr("New Project"), "", tr("(*.echo)"));
		if (!projectName.isEmpty())
		{
			// 0.confirm path and file name
			Echo::String fullPath = projectName.toStdString().c_str();
			Echo::String filePath = Echo::PathUtil::GetFileDirPath(fullPath);
			Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath, false);

			// 1.create directory
			Echo::String newFilePath = filePath + fileName + "/";
			if (!Echo::PathUtil::IsDirExist(newFilePath))
			{
				Echo::PathUtil::CreateDir(newFilePath);

				// 2.copy file
                copyQtFile(":/project/project/blank.echo", newFilePath + "blank.echo");
                copyQtFile(":/project/project/Res/icon.png",   newFilePath + "Res/icon.png");

				// 3.rename
				Echo::String projectPathName = newFilePath + "blank.echo";
				Echo::String destProjectPathName = newFilePath + fileName + ".echo";
				if (Echo::PathUtil::IsFileExist(projectPathName))
					Echo::PathUtil::RenameFile(projectPathName, destProjectPathName);

				return destProjectPathName;
			}
			else
			{
				EchoLogError("[%s] has existed", newFilePath.c_str());
			}
		}

		return "";
	}

	void ProjectWnd::openProject(const Echo::String& projectFile)
	{
		AStudio::instance()->getMainWindow()->showMaximized();
		AStudio::instance()->OpenProject(projectFile.c_str());
		AStudio::instance()->getRenderWindow();

		close();
	}

	void ProjectWnd::onDoubleClicked(const QString& name)
	{
		Echo::String projectName = name.toStdString().c_str();
		if (Echo::PathUtil::IsFileExist(projectName))
		{
			AStudio::instance()->getMainWindow()->showMaximized();
			AStudio::instance()->OpenProject(name.toStdString().c_str());
			AStudio::instance()->getRenderWindow();

			close();
		}
		else
		{
			Echo::String alertMsg = Echo::StringUtil::Format("project file [%s] not found", projectName.c_str());

			QMessageBox msgBox;
			msgBox.setText(alertMsg.c_str());
			msgBox.setIcon(QMessageBox::Critical);
			msgBox.exec();
		}
	}

    void ProjectWnd::onRemoveProject()
    {
        if(!m_selectedProject.empty())
        {
            EditorConfig::instance()->removeRencentProject(m_selectedProject.c_str());
            
            loadAllRecentProjects();
        }
    }

	void ProjectWnd::onShowProjectInExplorer()
	{
		QString openDir = Echo::PathUtil::GetFileDirPath(m_selectedProject).c_str();
		if (!openDir.isEmpty())
		{
#ifdef ECHO_PLATFORM_WINDOWS
			QDesktopServices::openUrl(openDir);
#else
			QDesktopServices::openUrl(QUrl("file://" + openDir));
#endif
		}
	}

    void ProjectWnd::showMenu(const QPoint& point)
    {
        QStandardItem* item = m_previewerWidget->itemAt( point);
        if(item)
        {
            m_selectedProject = item->data(Qt::UserRole).toString().toStdString().c_str();
            
            EchoSafeDelete(m_projectMenu, QMenu);
            m_projectMenu = EchoNew(QMenu);
            
            m_projectMenu->addAction(m_actionRemoveProject);
			m_projectMenu->addSeparator();
			m_projectMenu->addAction(m_actionShowInExplorer);

            m_projectMenu->exec(QCursor::pos());
        }
    }

	void ProjectWnd::onClicked(const QString& name)
	{
		Echo::String msg = name.toStdString().c_str();

		m_statusBar->showMessage(name);
	}

	void ProjectWnd::showAllUpdateableVersion()
	{
		Studio::Update updater;
		Echo::StringArray allVersions = updater.getAllEnabledVersions();
		for (Echo::String& version : allVersions)
		{
			QListWidgetItem* item = new QListWidgetItem(version.c_str());
			m_versionListWidget->addItem(item);
		}
	}

	// QTBUG-39220
	void ProjectWnd::showEvent(QShowEvent* event)
	{
		setAttribute(Qt::WA_Mapped);
		QMainWindow::showEvent(event);
	}

	void ProjectWnd::onDownloadNewVersion(QListWidgetItem* item)
	{
		Echo::String resName = item->text().toStdString().c_str();
		
		Studio::Update updater;
		updater.downloadVersion(resName);

		close();
	}
}
