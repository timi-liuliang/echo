#pragma once

#include <QListView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

namespace QT_UI
{
	class QPreviewHelper
	{
	public:
		QPreviewHelper(QListView* view);

		// add item
		void setPath(const char* filePath);

		// clear all items
		void clear();

		// when resize list view
		void onListViewResize();

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

	private:
		QListView*				m_listView;
		QStandardItemModel*		m_listModel;
		QSortFilterProxyModel*	m_listProxyModel;
	};
}