#include "PathChooseDialog.h"
#include "Studio.h"
#include <QPushButton>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Engine.h>

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

		// 目录树型结构
		m_dirModel = new QT_UI::QDirectoryModel();
		m_dirModel->SetIcon("root", QIcon(":/icon/Icon/root.png"));
		m_dirModel->SetIcon("filter", QIcon(":/icon/Icon/folder_close.png"));
		m_dirModel->SetIcon("filterexpend", QIcon(":/icon/Icon/folder_open.png"));
		m_resDirView->setModel(m_dirModel);
        m_resDirView->setAttribute(Qt::WA_MacShowFocusRect,0);
		m_dirModel->Clean();

		QStringList titleLable;
		titleLable << "Res://";
		m_dirModel->setHorizontalHeaderLabels(titleLable);

		m_dirModel->SetRootPath(Echo::Engine::instance()->getResPath().c_str(), "none", m_resDirView, NULL);
		m_dirModel->Refresh();

		// show exts
		m_comboBoxExts->addItem(Echo::StringUtil::Format("(%s)",exts).c_str());

		// 消息链接
		QObject::connect(m_dirModel, SIGNAL(FileSelected(const char*)), this, SLOT(onSelectDir(const char*)));
		QObject::connect(m_fileNameLine, SIGNAL(textChanged(const QString &)), this, SLOT(onFileNameChanged()));

		m_previewHelper = new QT_UI::QPreviewHelper(m_listView);
		QObject::connect(m_previewHelper, SIGNAL(doubleClickedRes(const char*)), this, SLOT(onDoubleClickPreviewRes(const char*)));

		// choose main directory
		onSelectDir(Echo::Engine::instance()->getResPath().c_str());
	}

	PathChooseDialog::~PathChooseDialog()
	{
		//AStudio::Instance()->getEchoEngine()->stopCurPreviewAudioEvent();
	}

	Echo::String PathChooseDialog::getSelectFile() const 
	{ 
		return m_selectedDir + m_fileNameLine->text().toStdString().c_str() + m_supportExts; 
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
			Echo::PathUtil::FormatPath(selectFile);

			return selectFile.c_str();
		}

		return "";
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
			Echo::String fileName = Echo::PathUtil::GetPureFilename(res, false);
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
