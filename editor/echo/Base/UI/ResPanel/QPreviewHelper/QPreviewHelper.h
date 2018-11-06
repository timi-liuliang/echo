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

		// set filter
		void setFilterPattern(const char* pattern);

		// clear all items
		void clear();

		// when resize list view
		void onListViewResize();

		// item at
		QStandardItem* itemAt(const QPoint& pos);

		// edit item
		void editItem(QStandardItem* item);

		// set mode
		void setUseIconMode();

		// set use list Mode
		void setUseListMode();

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

		// is support this ext
		bool isSupportExt(const Echo::String& file);

		// addToolTip
		void addToolTips(QStandardItem* item, const Echo::String& fullPath);

		// set item size hint
		void setItemSizeHint(int width, int height);

	private slots:
		// clicked resource
		void onClicked(const QModelIndex& pIndex);

		// double clicked resource
		void onDoubleClicked(const QModelIndex& pIndex);

		// rename res
		void renameRes( QStandardItem* item);

	private:
		int						m_iconSize;
		int						m_itemWidth;
		int						m_itemHeight;
		QListView*				m_listView;
		QStandardItemModel*		m_listModel;
		QSortFilterProxyModel*	m_listProxyModel;
		Echo::StringArray		m_supportExts;
	};
}