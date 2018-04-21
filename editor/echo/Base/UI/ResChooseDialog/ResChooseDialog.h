#pragma once

#include <QDialog>
#include <engine/core/util/LogManager.h>
#include "ui_ResChooseDialog.h"

namespace Studio
{
	class ResChooseDialog : public QDialog, public Ui_ResChooseDialog
	{
	public:
		ResChooseDialog(QWidget* parent = 0, const char* exts = 0, const char* filesFilter = 0, const char* startPath=0, bool chooseDir = false);
		~ResChooseDialog(){}

		// get file
		static Echo::String getExistingFile(QWidget* parent, const char* exts, const char* filesFilter=nullptr, const char* startPath=nullptr);

		// get directory
		static Echo::String getExistingDirectory(QWidget* parent);

		// get select file
		QString getSelectedFile();
	};
}