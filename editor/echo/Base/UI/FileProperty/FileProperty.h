#pragma once

#include <vector>
#include <QDialog>
#include <engine/core/Util/StringUtil.h>
#include "ui_FileProperty.h"

namespace Studio
{
	/**
	* 文件属性显示对话框
	*/
	class FilePropertyDialog : public QDialog
	{
		Q_OBJECT

	public:
		FilePropertyDialog(QWidget* parent = 0);
		virtual ~FilePropertyDialog();

		// 设置文件名
		void setFile( const char* fileName);

	private:
		Ui_FileProperty* m_ui;		// ui初始配置
	};
}