#pragma once

#include <QDialog>
#include <engine/core/Util/StringUtil.h>
#include "ui_PathChooseDialog.h"
#include "QDirectoryModel.h"

namespace Ui {
	class PathChooseDialog;
} // namespace Ui

namespace Studio
{
	/**
	 * 资源选择对话框
	 */
	class PathChooseDialog : public QDialog, public Ui_PathChooseDialog
	{
		Q_OBJECT

	public:
		PathChooseDialog( QWidget* parent=0);
		virtual ~PathChooseDialog();

		// 获取文件路径
		static QString getExistingPath( QWidget* parent);

		// 获取名称加路径
		static QString getExistingPathName(QWidget* parent,const char* ext);

	private:
		void setFileNameVisible(bool _val);

	public slots:

	void OnSelectFile(const char* path);

	private:	
		QT_UI::QDirectoryModel*	m_model;		// 目录
	};
}