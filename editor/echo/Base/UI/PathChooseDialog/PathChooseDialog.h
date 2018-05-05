#pragma once

#include <QDialog>
#include <engine/core/Util/StringUtil.h>
#include "ui_PathChooseDialog.h"
#include "QDirectoryModel.h"
#include "QPreviewHelper.h"

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
		Echo::String getSelectFile() const;

		// 获取文件路径
		static QString getExistingPath( QWidget* parent);

		// 获取名称加路径
		static QString getExistingPathName(QWidget* parent,const char* ext, const char* title);

	private:
		// set file name visible
		void setFileNameVisible(bool _val);

	public slots:
		// 选择文件夹
		void onSelectDir(const char* dir);

		// double click res
		void onDoubleClickPreviewRes(const char* res);

	private:
		QT_UI::QDirectoryModel*		m_dirModel;
		QT_UI::QPreviewHelper*		m_previewHelper;
		Echo::String				m_selectedDir;
		Echo::String				m_supportExts;
	};
}