#include "MainWindow.h"
#include "Studio.h"
#include "LogPanel.h"
#include "ResPanel.h"
#include "NodeTreePanel.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopservices>
#include <QShortcut>
#include <QMdiArea>
#include <QComboBox>
#include "DebuggerPanel.h"
#include "EchoEngine.h"
#include "QResSelect.h"
#include "ResChooseDialog.h"
#include "LuaEditor.h"
#include "BottomPanel.h"
#include "ProjectWnd.h"
#include "PathChooseDialog.h"
#include "RenderWindow.h"
#include "About.h"
#include <QTimer>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include <engine/core/scene/render_node.h>

namespace Studio
{
	static MainWindow* g_inst = nullptr;

	MainWindow::MainWindow(QMainWindow* parent/*=0*/)
		: QMainWindow( parent)
		, m_renderPanel(nullptr)
		, m_resPanel(nullptr)
		, m_gameProcess(nullptr)
		, m_scriptEditorPanel(nullptr)
		, m_aboutWindow(nullptr)
	{
		setupUi( this);

		// hide title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// set menu icon
		menubar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		// project operate
		QObject::connect(m_actionNewProject, SIGNAL(triggered(bool)), this, SLOT(onNewAnotherProject()));
		QObject::connect(m_actionOpenProject, SIGNAL(triggered(bool)), this, SLOT(onOpenAnotherProject()));
		QObject::connect(m_actionSaveProject, SIGNAL(triggered(bool)), this, SLOT(onSaveProject()));
		QObject::connect(m_actionSaveAsProject, SIGNAL(triggered(bool)), this, SLOT(onSaveasProject()));

		// connect scene operate signal slot
		QObject::connect(m_actionNewScene, SIGNAL(triggered(bool)), this, SLOT(onNewScene()));
		QObject::connect(m_actionSaveScene, SIGNAL(triggered(bool)), this, SLOT(onSaveScene()));
		QObject::connect(m_actionSaveAsScene, SIGNAL(triggered(bool)), this, SLOT(onSaveAsScene()));

		// connect signal slot
		QObject::connect(m_actionPlayGame, SIGNAL(triggered(bool)), this, SLOT(onPlayGame()));
		QObject::connect(m_actionStopGame, SIGNAL(triggered(bool)), &m_gameProcess, SLOT(terminate()));
		QObject::connect(m_actionExitEditor, SIGNAL(triggered(bool)), this, SLOT(close()));
		QObject::connect(m_actionApi, SIGNAL(triggered(bool)), this, SLOT(onOpenHelpDialog()));
		QObject::connect(m_actionDocumentation, SIGNAL(triggered(bool)), this, SLOT(onOpenWiki()));
		QObject::connect(m_actionAbout, SIGNAL(triggered(bool)), this, SLOT(onAbout()));

		// add combox, switch 2D,3D,Script etc.
		m_subEditComboBox = new QComboBox(m_toolBar);
		m_subEditComboBox->addItem("2D");
		m_subEditComboBox->addItem("3D");
		m_toolBar->addWidget(m_subEditComboBox);
		QObject::connect(m_subEditComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(onSubEditChanged(const QString&)));

		EchoAssert(!g_inst);
		g_inst = this;
	}

	MainWindow::~MainWindow()
	{
	}

	MainWindow* MainWindow::instance()
	{
		return g_inst;
	}

	void MainWindow::onOpenProject()
	{
		//setCentralWidget(nullptr);
		centralWidget()->setMaximumHeight(0);

		m_renderPanel = EchoNew(QDockWidget(this));
		m_resPanel = EchoNew(ResPanel(this));
		m_scenePanel = EchoNew(NodeTreePanel(this));
		m_bottomPanel = EchoNew(BottomPanel(this));
		m_scriptEditorPanel = EchoNew( LuaEditor(this));
		m_scriptEditorPanel->setVisible(false);
			
		// add renderWindow to RenderDockWidget
		QWidget* renderWindow = AStudio::instance()->getRenderWindow();
		m_renderPanel->setWidget(renderWindow);
		m_renderPanel->setWindowTitle("Render");

		this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
		this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
		this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
		this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

		this->addDockWidget(Qt::TopDockWidgetArea, m_scriptEditorPanel);
		this->addDockWidget(Qt::TopDockWidgetArea, m_renderPanel);
		this->addDockWidget(Qt::LeftDockWidgetArea, m_resPanel);
		this->addDockWidget(Qt::RightDockWidgetArea, m_scenePanel);
		this->addDockWidget(Qt::BottomDockWidgetArea, m_bottomPanel);

		m_resPanel->onOpenProject();

		// menu [Project(P)]
		updateRencentProjectsDisplay();
		updateSettingDisplay();

		// settings of echo
		QTimer::singleShot(100, this, SLOT(recoverEditSettings()));

		// signals & slots
		QObject::connect(m_actionSaveProject, SIGNAL(triggered(bool)), m_scriptEditorPanel, SLOT(save()));
		QObject::connect(m_scriptEditorPanel, SIGNAL(visibilityChanged(bool)), this, SLOT(onScriptEditVisibleChanged()));
		QObject::connect(m_renderPanel, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)), this, SLOT(onDockWidgetLocationChanged()));
	}

	// recover edit settings
	void MainWindow::recoverEditSettings()
	{
		// open last edit node tree
		Echo::String lastNodeTreePath = AStudio::instance()->getConfigMgr()->getValue("last_edit_node_tree");
		if (Echo::IO::instance()->isResourceExists(lastNodeTreePath))
		{
			openNodeTree(lastNodeTreePath);
		}

		// recover last edit type 
		Echo::String subEditType = AStudio::instance()->getConfigMgr()->getValue("main_window_sub_edit_type");
		if (!subEditType.empty())
		{
			m_subEditComboBox->setCurrentText( subEditType.c_str());
		}

		// recover last edit script
		Echo::String isScriptEditVisible = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_visible");
		Echo::String currentEditLuaScript = AStudio::instance()->getConfigMgr()->getValue("luascripteditor_current_file");
		if (isScriptEditVisible == "true" && !currentEditLuaScript.empty())
		{
			openLuaScript(currentEditLuaScript);
		}

		// save config
		m_resPanel->recoverEditSettings();
	}

	// new scene
	void MainWindow::onNewScene()
	{
		onSaveProject();
		m_scenePanel->clear();
		EchoEngine::instance()->newEditNodeTree();
	}

	// on save scene
	void MainWindow::onSaveScene()
	{
		onSaveProject();
	}

	// on save as scene
	void MainWindow::onSaveAsScene()
	{
		Echo::String savePath = PathChooseDialog::getExistingPathName(this, ".scene", "Save").toStdString().c_str();
		if (!savePath.empty() && !Echo::PathUtil::IsDir(savePath))
		{
			Echo::String resPath;
			if (Echo::IO::instance()->covertFullPathToResPath(savePath, resPath))
				EchoEngine::instance()->saveCurrentEditNodeTreeAs(resPath.c_str());

			// refresh respanel display
			m_resPanel->reslectCurrentDir();
		}
	}

	// new
	void MainWindow::onNewAnotherProject()
	{
		Echo::String newProjectPathName = AStudio::instance()->getProjectWindow()->newProject();
		if (!newProjectPathName.empty())
		{
			openAnotherProject(newProjectPathName);
		}
	}

	void MainWindow::onOpenAnotherProject()
	{
		QString projectName = QFileDialog::getOpenFileName(this, tr("Open Project"), "", tr("(*.echo)"));
		if (!projectName.isEmpty())
		{
			openAnotherProject(projectName.toStdString().c_str());
		}
	}

	// open another project
	void MainWindow::openAnotherProject(const Echo::String& fullPathName)
	{
		Echo::String app = QCoreApplication::applicationFilePath().toStdString().c_str();
		Echo::String cmd = Echo::StringUtil::Format("%s %s", app.c_str(), fullPathName.c_str());

		QProcess process;
		process.startDetached(cmd.c_str());

		// exit
		close();
	}

	void MainWindow::onSaveasProject()
	{
		Echo::String projectName = QFileDialog::getSaveFileName(this, tr("Save as Project"), "", tr("(*.echo)")).toStdString().c_str();
		if (!projectName.empty())
		{
			// 0.confirm path and file name
			Echo::String fullPath = projectName;
			Echo::String filePath = Echo::PathUtil::GetFileDirPath(fullPath);
			Echo::String fileName = Echo::PathUtil::GetPureFilename(fullPath, false);
			Echo::String saveasPath = filePath + fileName + "/";

			if (!Echo::PathUtil::IsDirExist(saveasPath))
			{
				Echo::PathUtil::CreateDir(saveasPath);

				Echo::String currentProject = Echo::IO::instance()->getFullPath(Echo::GameSettings::instance()->getPath());
				Echo::String currentPath = Echo::PathUtil::GetFileDirPath(currentProject);
				Echo::String currentName = Echo::PathUtil::GetPureFilename(currentProject, true);

				// copy resource
				Echo::PathUtil::CopyDir(currentPath, saveasPath);

				// rename
				Echo::String currentPathname = saveasPath + currentName;
				Echo::String destPathName = saveasPath + fileName + ".echo";
				Echo::PathUtil::RenameFile(currentPathname, destPathName);

				// open dest folder
				QString qSaveasPath = saveasPath.c_str();
				QDesktopServices::openUrl(qSaveasPath);
			}
			else
			{
				EchoLogError("Save as directory [%s] isn't null", saveasPath.c_str());
			}
		}
	}

	// Open rencent project
	void MainWindow::onOpenRencentProject()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String text = action->text().toStdString().c_str();
			if (Echo::PathUtil::IsFileExist(text))
				openAnotherProject(text);
			else
				EchoLogError("Project file [%s] not exist.", text.c_str());
		}
	}

	void MainWindow::onEditSingletonSettings()
	{
		QAction* action = qobject_cast<QAction*>(sender());
		if (action)
		{
			Echo::String text = action->text().toStdString().c_str();
			Echo::Object* obj = Echo::Class::create(text);
			if (obj)
				Studio::NodeTreePanel::instance()->setNextEditObject(obj);
		}
	}

	void MainWindow::setSubEdit(const char* subEditName)
	{
		m_subEditComboBox->setCurrentText(subEditName);
	}

	// sub editor operate
	void MainWindow::onSubEditChanged(const QString& subeditName)
	{
		Echo::String renderType = subeditName.toStdString().c_str();
		if (renderType == "2D")
		{
			Echo::Render::setRenderTypes(Echo::Render::Type_2D);

			RenderWindow* renderWindow = ECHO_DOWN_CAST<RenderWindow*>(AStudio::instance()->getRenderWindow());
			if (renderWindow)
				renderWindow->switchToController2d();

			AStudio::instance()->getConfigMgr()->setValue("main_window_sub_edit_type", subeditName.toStdString().c_str());
		}
		else if (renderType == "3D")
		{ 
			Echo::Render::setRenderTypes(Echo::Render::Type_3D);

			RenderWindow* renderWindow = ECHO_DOWN_CAST<RenderWindow*>(AStudio::instance()->getRenderWindow());
			if (renderWindow)
				renderWindow->switchToController3d();

			AStudio::instance()->getConfigMgr()->setValue("main_window_sub_edit_type", subeditName.toStdString().c_str());
		}	
	}

	void MainWindow::onSaveProject()
	{
		// if path isn't exist. choose a save directory
		if (EchoEngine::instance()->getCurrentEditNode() && EchoEngine::instance()->getCurrentEditNodeSavePath().empty())
		{
			Echo::String savePath = PathChooseDialog::getExistingPathName(this, ".scene", "Save").toStdString().c_str();
			if (!savePath.empty() && !Echo::PathUtil::IsDir(savePath))
			{
				Echo::String resPath;
				if (Echo::IO::instance()->covertFullPathToResPath(savePath, resPath))
					EchoEngine::instance()->setCurrentEditNodeSavePath(resPath.c_str());
			}
		}

		// save settings
		Echo::Engine::instance()->saveSettings();

		// save current edit node
		EchoEngine::instance()->saveCurrentEditNodeTree();

		// save current edit res
		m_scenePanel->saveCurrentEditRes();
		
		// refresh respanel display
		m_resPanel->reslectCurrentDir();
	}

	// play game
	void MainWindow::onPlayGame()
	{
		// if launch scene not exist, set it
		Echo::GameSettings* projSettings = Echo::GameSettings::instance();
		if (projSettings)
		{
			const Echo::String& launchScene = projSettings->getLaunchScene().getPath();
			if (launchScene.empty())
			{
				if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "Warning", "Launch Scene is empty, Would you set it now?", QMessageBox::Yes | QMessageBox::No).exec())
				{
					Echo::String  scene = ResChooseDialog::getExistingFile(this, ".scene");
					if (!scene.empty())
					{
						projSettings->setLaunchScene(Echo::ResourcePath(scene));
					}
					else
					{
						return;
					}
				}
				else
				{
					return;
				}
			}
		}

		// save project
		onSaveProject();

		// start game
		if (!projSettings->getLaunchScene().getPath().empty())
		{
			Echo::String app = QCoreApplication::applicationFilePath().toStdString().c_str();
			Echo::String project = Echo::Engine::instance()->getConfig().m_projectFile;
			Echo::String cmd = Echo::StringUtil::Format("%s play %s", app.c_str(), project.c_str());

			m_gameProcess.terminate();
			m_gameProcess.waitForFinished();

			m_gameProcess.start(cmd.c_str());

			QObject::connect(&m_gameProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onGameProcessFinished(int, QProcess::ExitStatus)));
			QObject::connect(&m_gameProcess, SIGNAL(readyRead()), this, SLOT(onReadMsgFromGame()));

			EchoLogWarning("**start game debug [%s]**", cmd.c_str());
		}
	}

	void MainWindow::OpenProject(const char* projectName)
	{
		AStudio::instance()->OpenProject(projectName);

		// init render window
		AStudio::instance()->getRenderWindow();
	}

	// open node tree
	void MainWindow::openNodeTree(const Echo::String& resPath)
	{
		// clear
		onNewScene();

		Studio::EchoEngine::instance()->onOpenNodeTree(resPath);

		NodeTreePanel::instance()->refreshNodeTreeDisplay();
		m_renderPanel->setWindowTitle( resPath.c_str());
	}

	// open lua file for edit
	void MainWindow::openLuaScript(const Echo::String& fileName)
	{
		m_scriptEditorPanel->open(fileName);
		m_scriptEditorPanel->setVisible(true);
	}

	// on display script edit panel
	void MainWindow::onScriptEditVisibleChanged()
	{
		if (m_scriptEditorPanel->isVisible())
		{
			resizeDocks({ m_scriptEditorPanel, m_renderPanel }, { 70 , 30 }, Qt::Horizontal);
		}

		AStudio::instance()->getConfigMgr()->setValue("luascripteditor_visible", m_scriptEditorPanel->isVisible() ? "true" : "false");
		AStudio::instance()->getConfigMgr()->setValue("luascripteditor_current_file", m_scriptEditorPanel->getCurrentLuaFilePath().c_str());
	}

	// on Dockwidget location changed
	void MainWindow::onDockWidgetLocationChanged()
	{
		//centralWidget()->setMaximumHeight(200);
		//centralWidget()->setMinimumHeight(200);
	}

	void MainWindow::closeEvent(QCloseEvent *event)
	{
		AStudio::instance()->getLogPanel()->close();

		// disconnect connections
		m_scriptEditorPanel->disconnect();
	}

	// game process exit
	void MainWindow::onGameProcessFinished(int id, QProcess::ExitStatus status)
	{
		EchoLogWarning("stop game debug");
	}

	// open help dialog
	void MainWindow::onOpenHelpDialog()
	{
		m_bottomPanel->setTabVisible( "DocumentPanel", !m_bottomPanel->isTabVisible("DocumentPanel"));
	}

	void MainWindow::onOpenWiki()
	{
		QString link = "https://github.com/blab-liuliang/echo/wiki";
		QDesktopServices::openUrl(QUrl(link));
	}

	void MainWindow::onAbout()
	{
		if (!m_aboutWindow)
		{
			m_aboutWindow = new AboutWindow(this);
			m_aboutWindow->setWindowModality(Qt::ApplicationModal);
		}

		m_aboutWindow->setVisible(true);
	}

	void MainWindow::onReadMsgFromGame()
	{
		Echo::String msg = m_gameProcess.readAllStandardOutput().toStdString().c_str();
		if (!msg.empty())
		{
			Echo::StringArray msgArray = Echo::StringUtil::Split(msg, "@@");
			
			int i = 0;
			int argc = msgArray.size();
			while (i < argc)
			{
				Echo::String command = msgArray[i++];
				if (command == "-log")
				{
					int    logLevel     = Echo::StringUtil::ParseInt(msgArray[i++]);
					Echo::String logMsg = msgArray[i++];

					Echo::Log::instance()->logMessage(Echo::LogOutput::Level(logLevel), logMsg.c_str());
				}
			}
		}
	}

	void MainWindow::updateRencentProjectsDisplay()
	{
		// clear
		m_menuRecents->clear();

		ConfigMgr* configMgr = AStudio::instance()->getConfigMgr();

		Echo::list<Echo::String>::type recentProjects;
		configMgr->getAllRecentProject(recentProjects);

		Echo::list<Echo::String>::iterator iter = recentProjects.begin();
		for (; iter != recentProjects.end(); ++iter)
		{
			Echo::String projectFullPath = (*iter).c_str();
			if (!projectFullPath.empty() && Echo::PathUtil::IsFileExist(projectFullPath))
			{
				Echo::String icon = Echo::PathUtil::GetFileDirPath(projectFullPath) + "icon.png";
				if (Echo::PathUtil::IsFileExist(icon))
				{
					QAction* action = new QAction(this);
					action->setText(projectFullPath.c_str());
					action->setIcon(QIcon(icon.c_str()));

					m_menuRecents->addAction(action);

					QObject::connect(action, SIGNAL(triggered()), this, SLOT(onOpenRencentProject()));
				}
			}
		}
	}

	void MainWindow::updateSettingDisplay()
	{
		m_menuSettings->clear();

		Echo::StringArray classes;
		Echo::Class::getAllClasses(classes);
		for (Echo::String& className : classes)
		{
			if (Echo::Class::isSingleton(className))
			{
				QAction* action = new QAction(this);
				action->setText(className.c_str());
				m_menuSettings->addAction(action);

				QObject::connect(action, SIGNAL(triggered()), this, SLOT(onEditSingletonSettings()));
			}
		}
	}
}