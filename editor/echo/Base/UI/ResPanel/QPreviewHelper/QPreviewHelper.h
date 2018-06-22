#pragma once

#include <QListView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	class QPreviewHelper : public QObject
	{
		Q_OBJECT

	public:
		QPreviewHelper(QListView* view);

		// add item
		void setPath(const Echo::String& filePath, const char* exts, bool includePreDir);

		// clear all items
		void clear();

		// when resize list view
		void onListViewResize();

		// item at
		QStandardItem* itemAt(const QPoint& pos);

		// edit item
		void editItem(QStandardItem* item);

	public: signals :
		// clicked
		void clickedRes(const char* path);

		// double clicked
		void doubleClickedRes(const char* path);

		// rename res
		void renamedRes(const QString srcPath, const QString destPath);

	private:
		// add item
		void addItem(const char* filePath);

		// create items
		void createItem(const char* filePath, std::vector<QStandardItem*>& results);

		// get icon
		QIcon QPreviewHelper::getFileIcon(const char* fullPath);

		// set mode
		void setUseIconMode();

		// set use list Mode
		void setUseListMode();

		// is support this ext
		bool isSupportExt(const Echo::String& file);

	private slots:
		// clicked resource
		void onClicked(const QModelIndex& pIndex);

		// double clicked resource
		void onDoubleClicked(const QModelIndex& pIndex);

		// rename res
		void renameRes( QStandardItem* item);

	private:
		QListView*				m_listView;
		QStandardItemModel*		m_listModel;
		QSortFilterProxyModel*	m_listProxyModel;
		Echo::StringArray		m_supportExts;
	};
}