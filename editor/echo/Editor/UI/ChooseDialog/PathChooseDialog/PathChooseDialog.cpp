#include "PathChooseDialog.h"
#include "Studio.h"
#include <QPushButton>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>
#include <engine/core/io/io.h>

namespace Studio
{
	PathChooseDialog::PathChooseDialog(QWidget* parent, const char* exts)
		: QDialog( parent)
		, m_supportExts(exts)
	{
		setupUi( this);

		// hide default window title
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

		// disable ok button
		m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

		// Tree Strict
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
        m_resDirView->setAttribute(Qt::WA_MacShowFocusRect,0);
		m_dirModel->Clean();

		// show exts
		m_comboBoxExts->addItem(Echo::StringUtil::Format("(%s)",exts).c_str());

		// connect signal slot
		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));
		QObject::connect(m_fileNameLine, SIGNAL(textChanged(const QString &)), this, SLOT(onFileNameChanged()));

		m_previewHelper = new QT_UI::QPreviewHelper(m_listView);
		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickPreviewRes(const char*)));

		setRootPaths();
	}

	PathChooseDialog::~PathChooseDialog()
	{
		//AStudio::Instance()->getEchoEngine()->stopCurPreviewAudioEvent();
	}

	Echo::String PathChooseDialog::getSelectFile() const 
	{ 
		return m_selectedDir + m_fileNameLine->text().toStdString().c_str(); 
	}

	Echo::String PathChooseDialog::getSelectPath() const
	{
		return m_selectedDir;
	}

	QString PathChooseDialog::getExistingPath(QWidget* parent)
	{
		QString selectFile;

		PathChooseDialog* dialog = new PathChooseDialog(parent, "none");
		dialog->setSelectingType(SelectingType::Folder);
		dialog->setWindowModality( Qt::WindowModal);
		dialog->show();
		if( dialog->exec()==QDialog::Accepted)
		{
			selectFile = dialog->getSelectPath().c_str();
		}

		delete dialog;
		return selectFile;
	}

	QString PathChooseDialog::getExistingPathName(QWidget* parent, const char* ext, const char* title)
	{
		PathChooseDialog dialog(parent, ext);
		dialog.setSelectingType(SelectingType::File);
		dialog.setWindowModality(Qt::WindowModal);
		dialog.setWindowTitle(title);
		if (dialog.exec() == QDialog::Accepted)
		{
			Echo::String selectFile = dialog.getSelectFile().c_str();
			Echo::String selectExt = Echo::PathUtil::GetFileExt(selectFile, false);
			if (selectExt.empty())
			{
				selectFile += ext;
			}

			Echo::PathUtil::FormatPath(selectFile);

			return selectFile.c_str();
		}

		return "";
	}

	void PathChooseDialog::setRootPaths()
	{
		// Make sure user path exist
		if (!Echo::PathUtil::IsDirExist(Echo::Engine::instance()->getUserPath()))
			Echo::PathUtil::CreateDir(Echo::Engine::instance()->getUserPath());

		// Set root paths
		QT_UI::QDirectoryModel::RootPathArray rootPaths =
		{
			{"Res://", Echo::IO::instance()->getResPath().c_str()},
			{"User://", Echo::IO::instance()->getUserPath().c_str()},
			{"Engine://", Echo::IO::instance()->getEngineResPath().c_str()}
		};

		m_dirModel->setRootPath(rootPaths, "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		// Choose main directory
		onSelectDir(Echo::Engine::instance()->getResPath().c_str());
	}

	void PathChooseDialog::setSelectingType(PathChooseDialog::SelectingType type)
	{
		m_selectingType = type;
		if (m_selectingType == SelectingType::File)
		{
			m_fileNameLine->setVisible(true);
			m_fileName->setVisible(true);
			m_labelType->setVisible(true);
			m_comboBoxExts->setVisible(true);
		}
		else
		{
			m_fileNameLine->setVisible(false);
			m_fileName->setVisible(false);
			m_labelType->setVisible(false);
			m_comboBoxExts->setVisible(false);
		}
	}

	void PathChooseDialog::onSelectDir(const char* dir)
	{
		bool isIncludePreDir = dir == Echo::Engine::instance()->getResPath() ? false : true;

		m_previewHelper->clear();
		m_previewHelper->setPath(dir, m_supportExts.c_str(), isIncludePreDir);

		m_selectedDir = dir;

		if (m_selectingType==Folder)
		{
			m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		}
	}

	void PathChooseDialog::onDoubleClickPreviewRes(const char* res)
	{
		if (Echo::PathUtil::IsDir(res))
		{
			m_dirModel->setCurrentSelect(res);
		}
		else
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(res, true);
			m_fileNameLine->setText( fileName.c_str());
		}
	}

	void PathChooseDialog::onFileNameChanged()
	{
		if (m_selectingType==File)
		{
			if (m_fileNameLine->text().isEmpty())
			{
				m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
			}
			else
			{
				m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
			}
		}
	}
}
