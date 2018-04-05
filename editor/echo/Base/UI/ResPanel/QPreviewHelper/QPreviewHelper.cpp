#include "QPreviewHelper.h"
#include <engine/core/util/StringUtil.h>
#include <engine/core/util/PathUtil.h>

namespace QT_UI
{
	QPreviewHelper::QPreviewHelper(QListView* view)	
		: m_listView(view)
	{
		setUseIconMode();

		m_listModel = new QStandardItemModel(m_listView);

		m_listProxyModel = new QSortFilterProxyModel(m_listView);
		m_listProxyModel->setSourceModel(m_listModel);
		m_listProxyModel->setFilterKeyColumn(0);

		m_listView->setModel(m_listProxyModel);
	}

	// add item
	void QPreviewHelper::setPath(const char* filePath)
	{
		// get all files
		Echo::StringArray files;
		Echo::PathUtil::EnumFilesInDir(files, filePath, true, false, true);

		// sort by name
		std::sort( files.begin(), files.end());

		// add directory
		for (const Echo::String& file : files)
		{
			if (Echo::PathUtil::IsDir(file))
				addItem(file.c_str());
		}

		// add files
		for (const Echo::String& file : files)
		{
			if (!Echo::PathUtil::IsDir(file))
				addItem(file.c_str());
		}
	}

	// add item
	void QPreviewHelper::addItem(const char* filePath)
	{
		std::vector<QStandardItem*> results;
		createItem(filePath, results);
		for (QStandardItem* item : results)
		{
			m_listModel->appendRow(item);
		}
	}

	// create items
	void QPreviewHelper::createItem(const char* filePath, std::vector<QStandardItem*>& results)
	{
		Echo::String previewFile = filePath;
		if (Echo::PathUtil::IsDir(previewFile))
		{
			Echo::String folderName = Echo::PathUtil::GetLastDirName(previewFile);
			QStandardItem* item = new QStandardItem(QIcon(":/icon/Icon/root.png"), folderName.c_str());
			results.push_back(item);
		}
		else
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(previewFile, true);
			QStandardItem* item = new QStandardItem( getFileIcon(previewFile.c_str()), fileName.c_str());
			results.push_back(item);
		}
	}

	// 获取文件图标
	QIcon QPreviewHelper::getFileIcon(const char* fullPath)
	{
		Echo::String fileExt = Echo::PathUtil::GetFileExt(fullPath, true);
		if (fileExt == ".png")
		{
			QPixmap pixmap(fullPath);
			return QIcon(pixmap.scaled(QSize(64, 64)));
		}

		return QIcon(":/icon/Icon/file/file.png");
	}

	// clear all items
	void QPreviewHelper::clear()
	{
		m_listModel->clear();
	}

	// set mode
	void QPreviewHelper::setUseIconMode()
	{
		m_listView->setIconSize(QSize(64, 64));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::IconMode);
		m_listView->setMovement(QListView::Free);
		m_listView->setFlow(QListView::LeftToRight);
		//m_listView->setWrapping(false);
		m_listView->setWordWrap(true);
		m_listView->setSpacing(5);
	}

	// set use list Mode
	void QPreviewHelper::setUseListMode()
	{
		m_listView->setIconSize(QSize(30, 30));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::ListMode);
		m_listView->setMovement(QListView::Free);
		m_listView->setWordWrap(true);
		m_listView->setSpacing(0);
	}

	// when resize list view, modify spacing
	void QPreviewHelper::onListViewResize()
	{
		if (m_listView->viewMode() == QListView::IconMode)
		{
			float listViewWidth = m_listView->geometry().width() -26;
			float iconSizeWidth = m_listView->iconSize().width();
			int numberIcons = std::max<int>(listViewWidth / iconSizeWidth, 1);
			int spacing = std::max<int>(listViewWidth - numberIcons * iconSizeWidth, 0) / numberIcons / 2;

			m_listView->setSpacing(std::max<int>(spacing, 0));
		}
	}
}