#pragma once

#include "ui_ProjectWindow.h"
#include "QPreviewWidget.h"
#include <engine/core/util/StringUtil.h>

class QShowEvent;
namespace Studio
{
	class ProjectWnd : public QMainWindow, public Ui_ProjectWindow
	{
		Q_OBJECT

	public:
		ProjectWnd(QMainWindow* parent = 0);
		~ProjectWnd();
        
        // Refresh rencent projects display
        void loadAllRecentProjects();

		// show all updateable version
		void showAllUpdateableVersion();

		// new project file
		Echo::String newProject();

		// open project
		void openProject(const Echo::String& projectFile);
        
    private:
        // add one rencent project
        void addRecentProject(const char* projecet);

	protected:
        // show event
		virtual void showEvent(QShowEvent* event) override;

	private slots:
		// open new project
		void openNewProject(int index);

		// clicked
		void onClicked(const QString& name);
		void onDoubleClicked(const QString& name);
        
        // remove project
        void onRemoveProject();
		void onShowProjectInExplorer();

		// download
		void onDownloadNewVersion(QListWidgetItem* item);
        
        // node tree widget show menu
        void showMenu(const QPoint& point);

	private:
		QT_UI::QPreviewWidget*  m_previewerWidget;
        QMenu*                  m_projectMenu = nullptr;
        Echo::String            m_selectedProject;
	};
}
