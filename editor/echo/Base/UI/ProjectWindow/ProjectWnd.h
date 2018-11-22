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

		void addRecentProject(const char* projecet);

		// show all updateable version
		void showAllUpdateableVersion();

		// new project file
		Echo::String newProject();

		// open project
		void openProject(const Echo::String& projectFile);

	protected:
		virtual void showEvent(QShowEvent* event) override;

	private slots:
		// open new project
		void openNewProject(int index);

		void onDoubleClicked(const QString& name);

		void onClicked(const QString& name);

		// download
		void onDownloadNewVersion(QListWidgetItem* item);

	private:
		QT_UI::QPreviewWidget* m_previewerWidget; 
	};
}
