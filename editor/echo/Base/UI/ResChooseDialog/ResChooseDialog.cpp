#include "ResChooseDialog.h"
#include <engine/core/main/Root.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>

namespace Studio
{
	ResChooseDialog::ResChooseDialog(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath, bool chooseDir)
		: QDialog(parent)
		, m_supportExts(exts)
	{
		setupUi(this);

		// hide default window title
		//setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

		// 目录树型结构
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
		m_dirModel->Clean();

		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));

		m_previewHelper = new QT_UI::QPreviewHelper(m_listView);
		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickPreviewRes(const char*)));

		// initialize path
		m_dirModel->clear();

		QStringList titleLable;
		titleLable << "Res://";
		m_dirModel->setHorizontalHeaderLabels(titleLable);

		m_dirModel->SetRootPath(Echo::Root::instance()->getResPath().c_str(), "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		onSelectDir(Echo::Root::instance()->getResPath().c_str());
	}

	// get file
	Echo::String ResChooseDialog::getExistingFile(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath)
	{
		Echo::String selectFile;

		ResChooseDialog dialog(parent, exts, filesFilter, startPath);
		dialog.show();
		if (dialog.exec() == QDialog::Accepted)
		{
			selectFile = dialog.getSelectedFile();
		}

		return selectFile;
	}

	// get directory
	Echo::String ResChooseDialog::getExistingDirectory(QWidget* parent)
	{
		return "";
	}

	// get select file
	const Echo::String& ResChooseDialog::getSelectedFile()
	{
		return m_selectedFile;
	}

	// 选择文件夹
	void ResChooseDialog::onSelectDir(const char* dir)
	{
		bool isIncludePreDir = dir == Echo::Root::instance()->getResPath() ? false : true;

		m_previewHelper->clear();
		m_previewHelper->setPath(dir, m_supportExts.c_str(), isIncludePreDir);
	}

	// double click res
	void ResChooseDialog::onDoubleClickPreviewRes(const char* res)
	{
		if (Echo::PathUtil::IsDir(res))
		{
			m_dirModel->setCurrentSelect(res);
		}
		else
		{
			Echo::String resPath;
			if (Echo::IO::instance()->covertFullPathToResPath(res, resPath))
			{
				m_selectedFile = resPath.c_str();
			}

			accept();
		}
	}
}