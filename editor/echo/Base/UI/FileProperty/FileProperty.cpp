#include "FileProperty.h"
#include "Studio.h"
#include <QFileInfo>
#include <QDateTime>
#include <QTextCodec>
#include <qfiledialog.h>
#include <Engine/core/main/Engine.h>
#include "ui_FileProperty.h"

namespace Studio
{
	// 构造函数
	FilePropertyDialog::FilePropertyDialog(QWidget* parent)
		: QDialog(parent)
	{
		m_ui = new Ui_FileProperty;
		m_ui->setupUi(this);
	}

	// 析构函数
	FilePropertyDialog::~FilePropertyDialog()
	{
	}

	// 设置文件名
	void FilePropertyDialog::setFile(const char* fileName)
	{
		Echo::String fileFullName;// = EchoResourceManager->getFileLocation(fileName);

		QFileInfo inputFileInfo(fileFullName.c_str());
		if (inputFileInfo.isFile())
		{
			qint64 fileSize = inputFileInfo.size();
			fileSize = max((fileSize / 1024), 1);
			QDateTime lastModify = inputFileInfo.lastModified();
			Echo::String modifyTime = lastModify.toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str();

			m_ui->m_path->setText(fileFullName.c_str());
			m_ui->m_size->setText(Echo::StringUtil::Format("%dKB", fileSize).c_str());
			m_ui->m_modifyTime->setText(modifyTime.c_str());
		}
	}
}