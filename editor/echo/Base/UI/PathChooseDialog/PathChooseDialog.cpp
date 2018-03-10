#include "PathChooseDialog.h"
#include "Studio.h"
#include <Engine/Core.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Root.h>

namespace Studio
{
	// 构造函数
	PathChooseDialog::PathChooseDialog(QWidget* parent)
		: QDialog( parent)
	{
		setupUi( this);

		// 目录树型结构
		m_model = new QT_UI::QDirectoryModel();
		m_model->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_model->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_model->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_PathTreeView->setModel(m_model);

		m_model->Clean();

		QStringList titleLable;
		titleLable << "Paths";
		m_model->setHorizontalHeaderLabels(titleLable);

		using namespace Echo;
		ProjectFile& projectFile = Studio_ProjectMgr->GetProjectFile();
		Echo::vector<ProjectFile::ArchiveItem>::type& archives = projectFile.getArchives();
		for (size_t i = 0; i < archives.size(); i++)
		{
			Echo::String fullPath = Studio_ProjectMgr->GetRootPath() + archives[i].m_archiveValue;
			m_model->SetRootPath(fullPath.c_str(), "none", m_PathTreeView, NULL/*m_proxyModel*/);
			m_model->Refresh();

			m_CurPath->setText(fullPath.c_str());
		}

		// 消息链接
		QObject::connect(m_model, SIGNAL(FileSelected(const char*)), this, SLOT(OnSelectFile(const char*)));
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

		PathChooseDialog* dialog = new PathChooseDialog(parent);
		dialog->setFileNameVisible(false);
		dialog->setWindowModality( Qt::WindowModal);
		dialog->show();
		if( dialog->exec()==QDialog::Accepted)
		{
			selectFile = dialog->m_CurPath->text();
		}

		delete dialog;
		return selectFile;
	}

	QString PathChooseDialog::getExistingPathName(QWidget* parent, const char* ext)
	{
		Echo::String selectFile;

		PathChooseDialog* dialog = new PathChooseDialog(parent);
		dialog->setFileNameVisible(true);
		QString file_ext = ext;
		file_ext = "." + file_ext;
		dialog->m_fileExt->setText(file_ext);
		dialog->setWindowModality(Qt::WindowModal);
		dialog->show();
		if (dialog->exec() == QDialog::Accepted)
		{
			selectFile = dialog->m_CurPath->text().toStdString().c_str();
			if (selectFile.empty())
				selectFile = Echo::Root::instance()->getProjectFile()->getPath();

			Echo::PathUtil::FormatPath(selectFile);

			selectFile += (dialog->m_fileNameLine->text() + file_ext).toStdString().c_str();
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
		m_CurPath->setText(path);
	}

}