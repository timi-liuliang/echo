#include "Studio.h"
#include "MainWindow.h"
#include "BottomPanel.h"
#include "ProjectWnd.h"
#include "RenderWindow.h"
#include "ResChooseDialog.h"
#include "LogPanel.h"
#include "ConfigMgr.h"
#include <QtUiTools/QUiLoader>
#include <QFile>
#include <QLibrary>
#include <QMetaMethod>
#include <QSplitter>
#include "QPropertyModel.h"
#include <shellapi.h>
#include <engine/core/util/HashGenerator.h>
#include <engine/core/util/TimeProfiler.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>
#include "EchoEngine.h"

namespace Studio
{
	AStudio::AStudio()
		: m_logPanel(nullptr)
		, m_mainWindow(nullptr)
	{
		m_renderWindow = NULL;
		m_projectCfg = EchoNew( ConfigMgr);

		m_log = NULL;

		// set astudio as the echo editor
		Editor::setInstance(this);
	}

	AStudio::~AStudio()
	{
		Echo::Engine::instance()->destroy();
		EchoSafeDelete(m_logPanel, LogPanel);

		EchoSafeDelete(m_projectWindow, ProjectWnd);
		EchoSafeDelete(m_renderWindow, RenderWindow);
		EchoSafeDelete(m_projectCfg, ConfigMgr);
		EchoSafeDelete(m_mainWindow, MainWindow);
	}

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

		// 添加默认日志处理
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
		// 初始日志系统
		initLogSystem();

		// 启动主界面
		m_mainWindow = EchoNew( MainWindow);
		m_projectWindow = EchoNew( ProjectWnd);

		loadAllRecentProjects();

		// 启动日志面板
		m_logPanel = EchoNew(LogPanel( m_mainWindow));
		Echo::Log::instance()->addOutput(m_logPanel);
	}

	void AStudio::Close()
	{

	}

	void AStudio::loadAllRecentProjects()
	{
		Echo::list<Echo::String>::type recentProjects;
		m_projectCfg->getAllRecentProject(recentProjects);

		Echo::list<Echo::String>::iterator iter = recentProjects.begin();
		for ( ; iter != recentProjects.end(); ++iter )
		{
			m_projectWindow->addRecentProject((*iter).c_str());
		}
	}

	bool AStudio::isThumbnailExists(const Echo::String& name)
	{
		Echo::String appPath = AStudio::instance()->getAppPath();
		Echo::String fileFullName = Echo::StringUtil::Format("%sCache/thumbnail/%s.bmp", appPath, name);

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
		// 新建渲染窗口
		if ( !m_renderWindow )
		{
			// 新建EchoEngine
			TIME_PROFILE
			(
				EchoEngine::instance();
				ThumbnailMgr::instance();
			)

			// 新建渲染窗口
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

		// 初始化渲染窗口
		TIME_PROFILE(EchoEngine::SetProject(fileName);)

		// 通知主窗口
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
		Echo::PathUtil::FormatPath(m_appPath, true);

		// calculate root path
#ifdef ECHO_PLATFORM_WINDOWS
		m_rootPath = m_appPath + "../../../../";
#else
#endif
	}

	bool AStudio::deleteResource(const char* res)
	{

		return false;
	}

	bool AStudio::isResCanbeDeleted(const char* res)
	{
		return true;
	}

	bool AStudio::saveThumbnail(const Echo::String& fileName, int type /* = 0 */)
	{
		bool success = ThumbnailMgr::instance()->saveThumbnail(fileName, ThumbnailMgr::THUMBNAIL_TYPE(type));
		if ( success )
		{
			QString itemName;
			itemName = fileName.c_str();
		}
		return success;
	}

	Echo::String AStudio::getThumbnailPath(const Echo::String& filePath, bool needOldExt)
	{
		// 过滤掉后缀名，加上bmp
		Echo::String fileName = Echo::PathUtil::GetPureFilename(filePath, needOldExt);
		Echo::String appPath = Echo::PathUtil::GetCurrentDir();

		unsigned int projectHash = Echo::BKDRHash(Echo::Engine::instance()->getConfig().m_projectFile.c_str());
		Echo::String thumbnailPath = Echo::StringUtil::Format("%s/Cache/project_%d/thumbnail/%s.bmp", appPath.c_str(), projectHash, fileName.c_str());

		return thumbnailPath;
	}

	void AStudio::resetCamera(float diroffset)
	{
		//auto* renderWindow = static_cast<RenderWindow*>(getRenderWindow());
		//renderWindow->getInputController()->onInitCameraSettings(diroffset);
	}

	void AStudio::showBottomPanel(Echo::BottomPanelTab* bottomPanel)
	{
		MainWindow::instance()->getBottomPanel()->showBottomPanel( bottomPanel);
	}

	// select a node object
	const Echo::String AStudio::selectANodeObject()
	{
		return Echo::StringUtil::BLANK;
	}

	// select a setting object
	const Echo::String AStudio::selectASettingObject()
	{
		return Echo::StringUtil::BLANK;
	}

	// select a resource object
	const Echo::String AStudio::selectAResObject()
	{
		Echo::String resPath = ResChooseDialog::getExistingFile( nullptr, "");
		return resPath;
	}
}