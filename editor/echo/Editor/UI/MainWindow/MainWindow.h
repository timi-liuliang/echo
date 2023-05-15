#pragma once

#include <QProcess>
#include <QTimer>
#include "ui_MainWindow.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/main/game_settings.h>
#include <QComboBox>
#include <QMdiArea>

namespace Studio
{
	class ResPanel;
	class NodeTreePanel;
	class TextEditor;
	class TextEditorArea;
	class ShaderEditor;
	class TimelinePanel;
	class LogPanel;
	class DocumentPanel;
	class DebuggerPanel;
	class BottomPanel;
	class AboutWindow;
	class MainWindow : public QMainWindow, public Ui_MainWindow
	{
		Q_OBJECT

		typedef Echo::vector<QDockWidget*>::type QDockWidgets;

	public:
		MainWindow(QMainWindow* parent=0);
		~MainWindow();

		// instance
		static MainWindow* instance();

		// call when open project
		void onOpenProject();
        
        // call before quit engine
        void onPrepareQuit();

		// open
		void OpenProject(const char* projectName);

		// open node tree
		void openNodeTree(const Echo::String& resPath);

		// open lua file for edit
		void openTextEditor(const Echo::String& resPath);
        
        // open shader editor
        void openShaderEditor(const Echo::String& resPath);

	public:
		// set theme
		static void setTheme(const char* theme);

	public:
		// add center panel
		void addCenterPanel(QDockWidget* panel, float widthRatio);
		void removeCenterPanel(QDockWidget* panel);

		// add bottom panel
		void addBottomPanel(QDockWidget* panel);
		void removeBottomPanel(QDockWidget* panel);

	protected:
		void closeEvent(QCloseEvent *event);

	public slots:
		// scene operate
		void onNewScene();
		void onSaveScene();
		void onSaveAsScene();
        
        // update render window title
        void updateRenderWindowTitle();

		// project operate
		void onNewAnotherProject();
		void onOpenAnotherProject();
		void onSaveProject();
		void onSaveasProject();
		void onOpenRencentProject();
		void onEditSingletonSettings();
        void onBuildProject();

		// sub editor operate
		void setSubEdit(const char* subEditName);
		void onSubEditChanged(const QString& subeditName);

		// change theme
		void onChooseTheme();
		void onChooseThemeDark();

		// game mode
		void onPlayGame();

		// Check launch scene
		bool checkLaunchSceneSetting(Echo::GameSettings* projSettings);

		// receive msg from game
		void onReadMsgFromGame();

		// game process exit
		void onGameProcessFinished(int id, QProcess::ExitStatus status);

		// open help dialog
		void onOpenHelpDialog();

		// open wiki page
		void onOpenWiki();

		// open about dialog
		void onAbout();

		// on display script edit panel
		void onScriptEditVisibilityChanged();
		void onCenterDockWidgetVisibilityChanged();
		void onExternalCenterDockWidgetVisibilityChanged();

		// on Dockwidget location changed
		void onDockWidgetLocationChanged();

		// open camera settings panel
		void onClickEditorCameraSettings();

		// show message
		void onShowStatusMessage();
        
    public slots:
		// window
		void onOpenWindow();

        // undo
        void onUndo();
        void onRedo();

		// save
		void onSave();

	protected slots:
		// open another project
		void openAnotherProject(const Echo::String& fullPathName);

		// update rencent project display
		void updateRencentProjectsDisplay();

		// update setting display
		void updateSettingDisplay();

		// recover edit settings
		void recoverEditSettings();

	private:
		// hide QTabbar white line
		void hideWhiteLineOfQTabBar();

	protected:
		QTimer				m_timer;
		QDockWidget*		m_renderPanel = nullptr;
		ResPanel*			m_resPanel = nullptr;
		NodeTreePanel*		m_nodeTreePanel = nullptr;
		TextEditorArea*	    m_scriptEditorMdiArea = nullptr;
		ShaderEditor*		m_shaderEditorPanel = nullptr;
		QDockWidgets		m_centerPanels;
		QDockWidgets		m_bottomPanels;
		LogPanel*			m_logPanel = nullptr;
		DocumentPanel*		m_documentPanel = nullptr;
		DebuggerPanel*		m_debuggerPanel = nullptr;
		QProcess			m_gameProcess;
		QComboBox*			m_subEditComboBox = nullptr;
		AboutWindow*		m_aboutWindow = nullptr;
		QMenu*				m_cameraSettingsMenu = nullptr;
	};
}
