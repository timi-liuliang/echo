#pragma once

#include <string>
#include <engine/core/log/LogOutput.h>
#include <engine/core/editor/editor.h>
#include "ProjectMgr.h"
#include "EditorConfig.h"
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
		void start();
		void close();

		// config manager
		EditorConfig* getConfigMgr() { return m_projectCfg; }
	
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
		void openProject( const char* fileName);

	public:
		// show bottom panel
		virtual void showBottomPanel(Echo::EditorDockPanel* centerPanel) override;

		// show center panel
		virtual void showCenterPanel(QDockWidget* panel, float widthRatio) override;
		virtual void removeCenterPanel(QDockWidget* panel) override;

	public:
		// select a class
		virtual const Echo::String selectAClass() override;

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

		// show object's property in property panel
		virtual void showObjectProperty(Echo::Object* object);

	private:
		AStudio();

		// init log system
		bool initLogSystem();

		// is thumbnail exist
		bool isThumbnailExists(const Echo::String& name);
        
        // init functions
        void initFunctions();

	private:
		Echo::String					m_appPath;
		MainWindow*						m_mainWindow = nullptr;
		ProjectWnd*						m_projectWindow = nullptr;
		RenderWindow*					m_renderWindow = nullptr;
		Echo::LogDefault*				m_log = nullptr;
		LogPanel*						m_logPanel = nullptr;
		EditorConfig*						m_projectCfg = nullptr;
	};
}

#define Studio_ProjectMgr Studio::AStudio::Instance()->getProjectMgr()
