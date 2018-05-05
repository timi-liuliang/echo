#include "PathChooseDialog.h"
#include "Studio.h"
#include <Engine/Core.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Root.h>

namespace Studio
{
	// 构造函数
	PathChooseDialog::PathChooseDialog(QWidget* parent, const char* exts)
		: QDialog( parent)
	{
		setupUi( this);

		// 目录树型结构
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_PathTreeView->setModel(m_dirModel);
		m_dirModel->Clean();

		QStringList titleLable;
		titleLable << "Res://";
		m_dirModel->setHorizontalHeaderLabels(titleLable);

		m_dirModel->SetRootPath(Echo::Root::instance()->getResPath().c_str(), exts, m_PathTreeView, NULL);
		m_dirModel->Refresh();

		// 消息链接
		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(OnSelectFile(const char*)));
	}

	// 析构函数
	PathChooseDialog::~PathChooseDialog()
	{
		//AStudio::Instance()->getEchoEngine()->stopCurPreviewAudioEvent();
	}

	// 获取文件
	QString PathChooseDialog::getExistingPath(QWidget* parent)
	{
		QString selectFile;

		PathChooseDialog* dialog = new PathChooseDialog(parent, "none");
		dialog->setFileNameVisible(false);
		dialog->setWindowModality( Qt::WindowModal);
		dialog->show();
		if( dialog->exec()==QDialog::Accepted)
		{
			selectFile = dialog->getSelectFile().c_str();
		}

		delete dialog;
		return selectFile;
	}

	QString PathChooseDialog::getExistingPathName(QWidget* parent, const char* ext)
	{
		Echo::String selectFile;

		PathChooseDialog* dialog = new PathChooseDialog(parent, ext);
		dialog->setFileNameVisible(true);
		dialog->m_fileExt->setText(ext);
		dialog->setWindowModality(Qt::WindowModal);
		dialog->show();
		if (dialog->exec() == QDialog::Accepted)
		{
			selectFile = dialog->getSelectFile().c_str();
			if (selectFile.empty())
				selectFile = Echo::Root::instance()->getProjectFile()->getPath();

			Echo::PathUtil::FormatPath(selectFile);

			selectFile += (dialog->m_fileNameLine->text() + ext).toStdString().c_str();
		}

		delete dialog;
		return selectFile.c_str();
	}

	void PathChooseDialog::setFileNameVisible(bool _val)
	{
		m_fileExt->setVisible(_val);
		m_fileNameLine->setVisible(_val);
		m_fileName->setVisible(_val);
	}

	void PathChooseDialog::OnSelectFile(const char* path)
	{
		m_selectedFile = path;
	}
}