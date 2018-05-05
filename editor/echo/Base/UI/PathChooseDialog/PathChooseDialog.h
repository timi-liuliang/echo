#pragma once

#include <QDialog>
#include <engine/core/Util/StringUtil.h>
#include "ui_PathChooseDialog.h"
#include "QDirectoryModel.h"

namespace Studio
{
	/**
	 * 资源选择对话框
	 */
	class PathChooseDialog : public QDialog, public Ui_PathChooseDialog
	{
		Q_OBJECT

	public:
		PathChooseDialog( QWidget* parent, const char* exts);
		virtual ~PathChooseDialog();

		// get select file
		const Echo::String& getSelectFile() const { return m_selectedFile; }

		// 获取文件路径
		static QString getExistingPath( QWidget* parent);

		// 获取名称加路径
		static QString getExistingPathName(QWidget* parent,const char* ext);

	private:
		// set file name visible
		void setFileNameVisible(bool _val);

	public slots:
		// select file slot
		void OnSelectFile(const char* path);

	private:
		Echo::String				m_selectedFile;
		QT_UI::QDirectoryModel*		m_dirModel;
	};
}