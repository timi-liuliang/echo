#include "Studio.h"
#include "MainWindow.h"
#include "BottomPanel.h"
#include "ProjectWnd.h"
#include "RenderWindow.h"
#include "ResChooseDialog.h"
#include "SettingChooseDialog.h"
#include "NodePathChooseDialog.h"
#include "PropertyChooseDialog.h"
#include "NodeTreePanel.h"
#include "LogPanel.h"
#include "ConfigMgr.h"
#include <QtUiTools/QUiLoader>
#include <QFile>
#include <QLibrary>
#include <QMetaMethod>
#include <QSplitter>
#include "QPropertyModel.h"
#include <engine/core/util/HashGenerator.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include "EchoEngine.h"
#include <QFileDialog>

namespace Studio
{
	AStudio::AStudio()
	{
		m_renderWindow = NULL;
		m_projectCfg = EchoNew( ConfigMgr);

		m_log = NULL;
        
        // init functions
        initFunctions();

		// set astudio as the echo editor
		Editor::setInstance(this);
	}

	AStudio::~AStudio()
	{
        using namespace Echo;
        
        // do something before quit
        m_mainWindow->onPrepareQuit();
        
        EchoSafeDeleteInstance(EchoEngine);
        EchoSafeDeleteInstance(Engine);
		EchoSafeDelete(m_logPanel, LogPanel);
		EchoSafeDelete(m_log, LogDefault);

		EchoSafeDelete(m_projectWindow, ProjectWnd);
		EchoSafeDelete(m_renderWindow, RenderWindow);
		EchoSafeDelete(m_projectCfg, ConfigMgr);
		EchoSafeDelete(m_mainWindow, MainWindow);	}

	AStudio* AStudio::instance()
	{
		static AStudio* inst = new AStudio;
		return inst;
	}

	const Echo::String& AStudio::getRootPath()
	{
		return m_rootPath;
	}

	bool AStudio::initLogSystem()
	{
		Echo::Engine::instance();

		// log config
		Echo::LogDefault::LogConfig logConfig;
		logConfig.logName = "echo.log";
		logConfig.logLevel = Echo::LogOutput::LL_INVALID;
		logConfig.path = "./cache/";
		logConfig.logFilename = "echo.log";
		logConfig.bFileOutput = true;

		Echo::Log::instance()->setOutputLeve(logConfig.logLevel);

		m_log = EchoNew(Echo::LogDefault(logConfig));
		if ( m_log )
			Echo::Log::instance()->addOutput(m_log);

		return true;
	}

	void AStudio::Start()
	{
		initLogSystem();

		m_mainWindow = EchoNew( MainWindow);
		m_projectWindow = EchoNew( ProjectWnd);
		m_projectWindow->loadAllRecentProjects();

		m_logPanel = EchoNew(LogPanel( m_mainWindow));
		Echo::Log::instance()->addOutput(m_logPanel);
	}

	void AStudio::Close()
	{

	}

	bool AStudio::isThumbnailExists(const Echo::String& name)
	{
		Echo::String appPath = AStudio::instance()->getAppPath();
		Echo::String fileFullName = Echo::StringUtil::Format("%sCache/thumbnail/%s.bmp", appPath.c_str(), name.c_str());

		return Echo::PathUtil::IsFileExist(fileFullName);
	}

	bool AStudio::replaceTraverseAllWidget(QWidget* parent, QWidget* from, QWidget* to)
	{
		if ( parent && parent->layout() )
		{
			auto replaced = parent->layout()->replaceWidget(from, to);
			if ( replaced )
			{
				delete replaced;
				return true;
			}
			auto widgets = parent->findChildren<QWidget*>();
			if ( !widgets.size() )
			{
				return false;
			}
			for ( int i = 0; i < widgets.size(); ++i )
			{
				if ( replaceTraverseAllWidget(widgets[i], from, to) )
				{
					return true;
				}
			}
		}
		return false;
	}

	RenderWindow* AStudio::getRenderWindow()
	{
		if ( !m_renderWindow )
		{
			TIME_PROFILE
			(
				EchoEngine::instance();
			)

			TIME_PROFILE
			(
				m_renderWindow = EchoNew(RenderWindow);
				m_renderWindow->BeginRender();
			)
		}

		return m_renderWindow;
	}

	void AStudio::setRenderWindowController(IRWInputController* controller)
	{
		RenderWindow* renderWindow = qobject_cast<RenderWindow*>(m_renderWindow);
		if ( renderWindow )
		{
			renderWindow->setInputController(controller);
		}
	}

	IRWInputController* AStudio::getRenderWindowController()
	{
		RenderWindow* renderWindow = qobject_cast<RenderWindow*>(m_renderWindow);
		if (renderWindow)
		{
			return renderWindow->getInputController();
		}
		else
			return nullptr;
	}

	QWidget* AStudio::getMainWindow()
	{
		return m_mainWindow;
	}

	ProjectWnd* AStudio::getProjectWindow()
	{
		return m_projectWindow;
	}

	void AStudio::OpenProject(const char* fileName)
	{
		// remember it
		m_projectCfg->addRecentProject(fileName);

		// set project
		TIME_PROFILE(EchoEngine::SetProject(fileName);)

		// notify main window
		TIME_PROFILE
		(
			m_mainWindow->onOpenProject();
			m_mainWindow->setWindowTitle(fileName);
		)

		// output time profiler
		TIME_PROFILE_OUTPUT
	}

	void AStudio::setAppPath(const char* appPath)
	{
		m_appPath = appPath;
		Echo::PathUtil::FormatPath(m_appPath, false);

		// calculate root path
#ifdef ECHO_PLATFORM_WINDOWS
		m_rootPath = m_appPath + "../../../../";
#else
        m_rootPath = m_appPath + "../../../../";
#endif

		Echo::Engine::instance()->setRootPath(m_rootPath);
	}

	void AStudio::showBottomPanel(Echo::PanelTab* bottomPanel)
	{
		MainWindow::instance()->getBottomPanel()->showPanel( bottomPanel);
	}

	void AStudio::showCenterPanel(QDockWidget* panel, float widthRatio)
	{
		MainWindow::instance()->addCenterPanel(panel, widthRatio);
	}

	void AStudio::removeCenterPanel(QDockWidget* panel)
	{
		MainWindow::instance()->removeCenterPanel(panel);
	}

	const Echo::String AStudio::selectANodeObject()
	{
		return NodePathChooseDialog::getSelectingNode( nullptr);
	}

	const Echo::String AStudio::selectASettingObject()
	{
		return SettingChooseDialog::getSelectingSetting(nullptr);
	}

	const Echo::String AStudio::selectAResObject(const char* exts)
	{
		Echo::String resPath = ResChooseDialog::getSelectingFile( nullptr, exts);
		return resPath;
	}

	const Echo::String AStudio::selectAProperty(Echo::Object* objectPtr)
	{
		return PropertyChooseDialog::getSelectingProperty(nullptr, objectPtr);
	}

	const Echo::String AStudio::selectAFile(const char* title, const char* exts)
	{
		QString resFile = QFileDialog::getOpenFileName(nullptr, title, "", exts);
		if (!resFile.isEmpty())
		{
			return resFile.toStdString().c_str();
		}

		return Echo::StringUtil::BLANK;
	}

	Echo::ImagePtr AStudio::getNodeIcon( Echo::Node* node)
	{
		return nullptr;
	}
}
