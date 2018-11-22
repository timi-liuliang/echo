#pragma once

#include <string>
#include <engine/core/log/LogOutput.h>
#include <engine/core/editor/editor.h>
#include "ProjectMgr.h"
#include "ConfigMgr.h"
#include "IInputController.h"
#include "Logic/ThumbnailMgr/ThumbnailMgr.h"
#include <QWidget>
#include <QDockWidget>

namespace Studio
{
	class MainWindow;
	class ProjectWnd;
	class RenderWindow;
	class LogPanel;
	class AStudio : public Echo::Editor
	{
	public:
		~AStudio();

		// instance
		static AStudio* instance();

		// 启动
		void Start();

		// 关闭
		void Close();

		// 设置程序工作路径
		void setAppPath( const char* appPath);

		// 获取程序工作路径
		const char* getAppPath() const { return m_appPath.c_str(); }

		// get editor root path
		const Echo::String& getRootPath();

		// 获取配置管理器
		ConfigMgr* getConfigMgr() { return m_projectCfg; }

		// 获取日志窗口
		LogPanel* getLogPanel() { return m_logPanel; }

		// 获取渲染窗口
		virtual RenderWindow* getRenderWindow();

		// 保存缩略图
		virtual bool saveThumbnail( const Echo::String& fileName, int type = 0 );

		// 根据文件名获取缩略图全路径
		Echo::String getThumbnailPath( const Echo::String& filePath,bool needOldExt = true);
		
		// 重置摄像机参数
		virtual void resetCamera(float diroffset = 0);

		// 设置渲染窗口控制器
		virtual void setRenderWindowController( IRWInputController* controller);

		virtual IRWInputController* getRenderWindowController();

	public:
		// 资源是否可被删除
		virtual bool isResCanbeDeleted( const char* res);

		// 删除资源
		virtual bool deleteResource(const char* res);

	public:
		// 获取主窗口
		QWidget* getMainWindow();
		ProjectWnd* getProjectWindow();
		QDockWidget* getPropertyPanel();

		// 打开项目文件
		void OpenProject( const char* fileName);

	public:
		// show bottom panel
		virtual void showBottomPanel(Echo::BottomPanelTab* bottomPanel) override;

	public:
		// select a node object
		virtual const Echo::String selectANodeObject()override;

		// select a setting object
		virtual const Echo::String selectASettingObject() override;

		// select a resource object
		virtual const Echo::String selectAResObject() override;

	private:
		AStudio();

		// 初始化日志系统
		bool initLogSystem();

		// 缩略图是否存在
		bool isThumbnailExists(const Echo::String& name);

		bool replaceTraverseAllWidget(QWidget* parent, QWidget* from, QWidget* to);

		void loadAllRecentProjects();

	private:
		Echo::String					m_rootPath;			// root direcory
		Echo::String					m_appPath;			// 程序运营目录
		MainWindow*						m_mainWindow;		// 主窗口
		ProjectWnd*						m_projectWindow;
		RenderWindow*					m_renderWindow;		// 渲染窗口
		Echo::LogDefault*				m_log;				// 日志实现
		LogPanel*						m_logPanel;			// 日志面板
		ConfigMgr*						m_projectCfg;		// 配置文件
	};
}

#define Studio_ProjectMgr Studio::AStudio::Instance()->getProjectMgr()
