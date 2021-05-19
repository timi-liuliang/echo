#pragma once

#include <QDialog>
#include <engine/core/util/StringUtil.h>
#include "ui_PathChooseDialog.h"
#include "QDirectoryModel.h"
#include "QPreviewHelper.h"

namespace Studio
{
	class PathChooseDialog : public QDialog, public Ui_PathChooseDialog
	{
		Q_OBJECT

	public:
		// Selecting type
		enum SelectingType
		{
			Folder,
			File,
		};

	public:
		PathChooseDialog( QWidget* parent, const char* exts);
		virtual ~PathChooseDialog();

		// result
		Echo::String getSelectFile() const;
		Echo::String getSelectPath() const;

		// get path
		static QString getExistingPath( QWidget* parent);

		// get existing path name
		static QString getExistingPathName(QWidget* parent,const char* ext, const char* title);

	private:
		// path
		void setRootPaths();

		// set type
		void setSelectingType(SelectingType type);

	public slots:
		// on select dir
		void onSelectDir(const char* dir);

		// double click res
		void onDoubleClickPreviewRes(const char* res);

		// on file name changed
		void onFileNameChanged();

	private:
		SelectingType				m_selectingType = SelectingType::File;
		QT_UI::QDirectoryModel*		m_dirModel;
		QT_UI::QPreviewHelper*		m_previewHelper;
		Echo::String				m_selectedDir;
		Echo::String				m_supportExts;
	};
}
