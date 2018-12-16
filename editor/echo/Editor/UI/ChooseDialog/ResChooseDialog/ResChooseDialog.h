#pragma once

#include <QDialog>
#include "ui_ResChooseDialog.h"
#include <QDirectoryModel.h>
#include "QPreviewHelper.h"

namespace Studio
{
	class ResChooseDialog : public QDialog, public Ui_ResChooseDialog
	{
		Q_OBJECT

	public:
		ResChooseDialog(QWidget* parent = 0, const char* exts = 0, const char* filesFilter = 0, const char* startPath=0, bool chooseDir = false);
		~ResChooseDialog(){}

		// get file
		static Echo::String getSelectingFile(QWidget* parent, const char* exts, const char* filesFilter=nullptr, const char* startPath=nullptr);

		// get select file
		const Echo::String& getSelectedFile();

	public slots:
		// Ñ¡ÔñÎÄ¼þ¼Ð
		void onSelectDir(const char* dir);

		// on click res
		void onClickPreviewRes(const char* res);

		// double click res
		void onDoubleClickPreviewRes(const char* res);

	private:
		QT_UI::QDirectoryModel*		m_dirModel;
		QT_UI::QPreviewHelper*		m_previewHelper;
		Echo::String				m_selectedFile;
		Echo::String				m_supportExts;
	};
}