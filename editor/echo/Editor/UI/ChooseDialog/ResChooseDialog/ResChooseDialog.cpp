#include "ResChooseDialog.h"
#include <engine/core/main/Engine.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/io/IO.h>

namespace Studio
{
	static Echo::String g_lastSelectDir;

	ResChooseDialog::ResChooseDialog(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath, bool chooseDir)
		: QDialog(parent)
		, m_supportExts(exts)
	{
		setupUi(this);

#ifdef ECHO_PLATFORM_WINDOWS
		// hide window hwnd
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
#elif defined(ECHO_PLATFORM_MAC)
		// set title bar color
		//macChangeTitleBarColor(winId(), 0.f, 0.f, 0.f);
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		m_menuBar->setNativeMenuBar(false);
#endif

		// set icon
		m_menuBar->setTopLeftCornerIcon(":/icon/Icon/icon.png");

		// directory model
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
        m_resDirView->setAttribute(Qt::WA_MacShowFocusRect,0);
		m_dirModel->Clean();

		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));

		m_previewHelper = new QT_UI::QPreviewHelper(m_listView);
		QObject::connect(m_previewHelper, SIGNAL(clickedRes(const char*)), this, SLOT(onClickPreviewRes(const char*)));
		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickPreviewRes(const char*)));

		// initialize path
		m_dirModel->clear();

		QStringList titleLable;
		titleLable << "Res://";
		m_dirModel->setHorizontalHeaderLabels(titleLable);

		m_dirModel->SetRootPath(Echo::Engine::instance()->getResPath().c_str(), "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		if(!g_lastSelectDir.empty())
			onSelectDir(g_lastSelectDir.c_str());
		else
			onSelectDir(Echo::Engine::instance()->getResPath().c_str());
	}

	Echo::String ResChooseDialog::getSelectingFile(QWidget* parent, const char* exts, const char* filesFilter, const char* startPath)
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

	const Echo::String& ResChooseDialog::getSelectedFile()
	{
		return m_selectedFile;
	}

	void ResChooseDialog::onSelectDir(const char* dir)
	{
		bool isIncludePreDir = dir == Echo::Engine::instance()->getResPath() ? false : true;

		m_previewHelper->clear();
		m_previewHelper->setPath(dir, m_supportExts.c_str(), isIncludePreDir);

		g_lastSelectDir = dir;
	}

	void ResChooseDialog::onClickPreviewRes(const char* res)
	{
		if (!Echo::PathUtil::IsDir(res))
		{
			Echo::String resPath;
			if (Echo::IO::instance()->convertFullPathToResPath(res, resPath))
			{
				m_selectedFile = resPath.c_str();
			}
		}
	}

	void ResChooseDialog::onDoubleClickPreviewRes(const char* res)
	{
		if (Echo::PathUtil::IsDir(res))
		{
			m_dirModel->setCurrentSelect(res);
		}
		else
		{
			Echo::ResPtr resPtr = Echo::Res::createByFileExtension(Echo::PathUtil::GetFileExt(res, true), true);
			if (resPtr && resPtr->isPackage())
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(res, resPath))
				{
					Echo::ResPtr selectRes = Echo::Res::get(resPath);
					m_previewHelper->clear();
					m_previewHelper->setRes(selectRes, m_supportExts.c_str(), true);
				}
			}
			else
			{
				Echo::String resPath;
				if (Echo::IO::instance()->convertFullPathToResPath(res, resPath))
				{
					m_selectedFile = resPath.c_str();
				}

				accept();
			}
		}
	}
}
