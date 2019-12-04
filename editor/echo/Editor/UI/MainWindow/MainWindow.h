#pragma once

#include <QProcess>
#include "ui_MainWindow.h"
#include <engine/core/util/StringUtil.h>
#include <QComboBox>
#include <QMdiArea>

namespace Studio
{
	class ResPanel;
	class NodeTreePanel;
	class LuaEditor;
	class ShaderEditor;
	class ScratchEditor;
	class TimelinePanel;
	class DebuggerPanel;
	class BottomPanel;
	class AboutWindow;
	class MainWindow : public QMainWindow, public Ui_MainWindow
	{
		Q_OBJECT

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
		void openLuaScript(const Echo::String& resPath);

	public:
		// set theme
		static void setTheme(const char* theme);

	public:
		// panels
		BottomPanel* getBottomPanel() { return m_bottomPanel; }

	protected:
		void closeEvent(QCloseEvent *event);

	public slots:
		// scene operate
		void onNewScene();
		void onSaveScene();
		void onSaveAsScene();

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
		void onScriptEditVisibleChanged();

		// on Dockwidget location changed
		void onDockWidgetLocationChanged();
        
    public slots:
        // undo
        void onUndo();
        void onRedo();

	protected slots:
		// open another project
		void openAnotherProject(const Echo::String& fullPathName);

		// update rencent project display
		void updateRencentProjectsDisplay();

		// update setting display
		void updateSettingDisplay();

		// recover edit settings
		void recoverEditSettings();

	protected:
		QDockWidget*	m_renderPanel;
		ResPanel*		m_resPanel;
		NodeTreePanel*	m_scenePanel;
		LuaEditor*		m_scriptEditorPanel = nullptr;
		ShaderEditor*	m_shaderEditorPanel = nullptr;
		ScratchEditor*	m_scratchEditorPanel = nullptr;
		BottomPanel*	m_bottomPanel;
		QProcess		m_gameProcess;
		QComboBox*		m_subEditComboBox;
		AboutWindow*	m_aboutWindow;
	};
}
