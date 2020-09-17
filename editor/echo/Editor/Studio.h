#pragma once

#include <string>
#include <engine/core/log/LogOutput.h>
#include <engine/core/editor/editor.h>
#include "ProjectMgr.h"
#include "ConfigMgr.h"
#include "IInputController.h"
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

		// start|close
		void Start();
		void Close();

		// app path
		void setAppPath( const char* appPath);
		const char* getAppPath() const { return m_appPath.c_str(); }

		// get editor root path
		const Echo::String& getRootPath();

		// config manager
		ConfigMgr* getConfigMgr() { return m_projectCfg; }
	
		// input controller
		virtual void setRenderWindowController( IRWInputController* controller);
		virtual IRWInputController* getRenderWindowController();

	public:
		// get main window
		QWidget* getMainWindow();
		ProjectWnd* getProjectWindow();
		QDockWidget* getPropertyPanel();

		// render window
		virtual RenderWindow* getRenderWindow();

		// log panel
		LogPanel* getLogPanel() { return m_logPanel; }


		// open project
		void OpenProject( const char* fileName);

	public:
		// show bottom panel
		virtual void showBottomPanel(Echo::PanelTab* centerPanel) override;

		// show center panel
		virtual void showCenterPanel(QDockWidget* panel, float widthRatio) override;
		virtual void removeCenterPanel(QDockWidget* panel) override;

	public:
		// select a node object
		virtual const Echo::String selectANodeObject()override;

		// select a setting object
		virtual const Echo::String selectASettingObject() override;

		// select a resource object
		virtual const Echo::String selectAResObject(const char* exts) override;

		// select a object's property
		virtual const Echo::String selectAProperty(Echo::Object* objectPtr) override;

		// select a file in os
		virtual const Echo::String selectAFile(const char* title, const char* exts) override;

	public:
		// get node icon by class name
		virtual Echo::ImagePtr getNodeIcon(Echo::Node* node) override;

	private:
		AStudio();

		// init log system
		bool initLogSystem();

		// is thumbnail exist
		bool isThumbnailExists(const Echo::String& name);

		bool replaceTraverseAllWidget(QWidget* parent, QWidget* from, QWidget* to);
        
        // init functions
        void initFunctions();

	private:
		Echo::String					m_rootPath;			// root directory
		Echo::String					m_appPath;
		MainWindow*						m_mainWindow = nullptr;
		ProjectWnd*						m_projectWindow = nullptr;
		RenderWindow*					m_renderWindow = nullptr;
		Echo::LogDefault*				m_log = nullptr;
		LogPanel*						m_logPanel = nullptr;
		ConfigMgr*						m_projectCfg = nullptr;
	};
}

#define Studio_ProjectMgr Studio::AStudio::Instance()->getProjectMgr()
