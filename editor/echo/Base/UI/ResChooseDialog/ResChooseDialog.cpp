#include "ResChooseDialog.h"

namespace Studio
{
	ResChooseDialog::ResChooseDialog(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath, bool chooseDir)
		: QDialog(parent)
	{
		setupUi(this);
	}

	// get file
	Echo::String ResChooseDialog::getExistingFile(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath)
	{
		QString selectFile;

		ResChooseDialog dialog(parent, exts, filesFilter, startPath);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			selectFile = dialog.getSelectedFile();
		}

		return selectFile.toStdString().c_str();
	}

	// get directory
	Echo::String ResChooseDialog::getExistingDirectory(QWidget* parent)
	{
		return "";
	}

	// get select file
	QString ResChooseDialog::getSelectedFile()
	{
		return "";
	}
}