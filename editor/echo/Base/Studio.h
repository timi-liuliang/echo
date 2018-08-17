#pragma once

#include <string>
#include <engine/core/log/LogOutput.h>
#include "ProjectMgr.h"
#include "ConfigMgr.h"
#include "IInputController.h"
#include "Logic/ThumbnailMgr/ThumbnailMgr.h"
#include <QWidget>
#include <QDockWidget>

namespace Studio
{
	// 文件支持
	struct AssetSupportInfo
	{
		std::string		m_icon;			// 图标
		std::string		m_tabName;		// Tab名称
		std::string		m_name;			// 资源名称
		std::string		m_ext;			// 资源名后缀

		AssetSupportInfo()
		{}

		AssetSupportInfo( const std::string& tab, const std::string& ext)
			: m_tabName(tab)
			, m_ext(ext)
		{}

		// 运算符重载 "=="
		bool operator == (const AssetSupportInfo& src) const
		{
			return (m_tabName == src.m_tabName) && (m_ext == src.m_ext);
		}
	};
	typedef std::vector<AssetSupportInfo> AssertsSupportInfoArray;

	class MainWindow;
	class ProjectWnd;
	class RenderWindow;
	class LogPanel;
	class AStudio
	{
	public:
		~AStudio();

		// 命令行模式初始化
		AStudio(const char* inputProject);

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

		// 获取配置管理器
		ConfigMgr* getConfigMgr() { return m_projectCfg; }

		// 获取日志窗口
		LogPanel* getLogPanel() { return m_logPanel; }

		// 获取渲染窗口
		virtual QWidget* getRenderWindow();

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

	private:
		AStudio();

		// 初始化日志系统
		bool initLogSystem();

		// 缩略图是否存在
		bool isThumbnailExists(const Echo::String& name);

		bool replaceTraverseAllWidget(QWidget* parent, QWidget* from, QWidget* to);

		void loadAllRecentProjects();

	private:
		Echo::String					m_appPath;			// 程序运营目录
		MainWindow*						m_mainWindow;		// 主窗口
		ProjectWnd*						m_projectWindow;
		RenderWindow*					m_renderWindow;		// 渲染窗口
		Echo::LogDefault*				m_log;				// 日志实现
		LogPanel*						m_logPanel;			// 日志面板
		ConfigMgr*						m_projectCfg;		// 配置文件

		std::vector<AssetSupportInfo>	m_assetSupportInfos;// 资源编辑相关信息
	};
}

#define Studio_ProjectMgr Studio::AStudio::Instance()->getProjectMgr()
