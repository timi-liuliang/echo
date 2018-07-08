#pragma once

#include "ui_ProjectWindow.h"
#include "QPreviewWidget.h"
#include <engine/core/util/StringUtil.h>

class QShowEvent;
namespace Studio
{
	/**
	 * 项目管理窗口
	 */
	class ProjectWnd : public QMainWindow, public Ui_ProjectWindow
	{
		Q_OBJECT

	public:
		ProjectWnd(QMainWindow* parent = 0);
		~ProjectWnd();

		void addRecentProject(const char* projecet);

		// 显示所有可更新版本
		void showAllUpdateableVersion();

		// new project file
		Echo::String newProject();

	protected:
		virtual void showEvent(QShowEvent* event) override;

	private slots:
		// 打开新项目
		void openNewProject(int index);

		void onDoubleClicked(const QString& name);

		void onClicked(const QString& name);

		// 点击下载
		void onDownloadNewVersion(QListWidgetItem* item);

	private:
		QT_UI::QPreviewWidget* m_previewerWidget; 
	};
}
