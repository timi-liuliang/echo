#pragma once

#include <QDockWidget>
#include "ui_ResPanel.h"
#include "QProperty.hpp"
#include <QDirectoryModel.h>
#include "QPreviewHelper.h"

namespace Studio
{
	class ResPanel : public QDockWidget, public Ui_ResPanel
	{
		Q_OBJECT

	public:
		ResPanel( QWidget* parent=0);
		~ResPanel();

		// call when open project
		void onOpenProject();

	public slots:
		// 选择文件夹
		void onSelectDir(const char* dir);

	private:
		QT_UI::QDirectoryModel*		m_dirModel;
		QT_UI::QPreviewHelper*		m_previewHelper;
	};
}