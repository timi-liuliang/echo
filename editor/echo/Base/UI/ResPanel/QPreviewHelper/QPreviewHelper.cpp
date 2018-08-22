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

		QObject::connect(m_listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
		QObject::connect(m_listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
		QObject::connect(m_listModel, &QStandardItemModel::itemChanged, this, &QPreviewHelper::renameRes);
	}

	// add item
	void QPreviewHelper::setPath(const Echo::String& filePath, const char* exts, bool includePreDir)
	{
		// get all files
		Echo::StringArray files;
		Echo::PathUtil::EnumFilesInDir(files, filePath, true, false, true);

		if (exts)
			m_supportExts = Echo::StringUtil::Split(exts, "|");
		else
			m_supportExts.clear();

		// include pre directory
		if (includePreDir)
		{
			addItem((filePath+"../").c_str());
		}

		// add directory
		for (const Echo::String& file : files)
		{
			if (Echo::PathUtil::IsDir(file))
				addItem(file.c_str());
		}

		// add files
		for (const Echo::String& file : files)
		{
			if (!Echo::PathUtil::IsDir(file) && isSupportExt(file))
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
			item->setData( previewFile.c_str(), Qt::UserRole);
			results.push_back(item);
		}
		else
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(previewFile, true);
			QStandardItem* item = new QStandardItem( getFileIcon(previewFile.c_str()), fileName.c_str());
			item->setData(previewFile.c_str(), Qt::UserRole);
			results.push_back(item);
		}
	}

	// 获取文件图标
	QIcon QPreviewHelper::getFileIcon(const char* fullPath)
	{
		Echo::String fileExt = Echo::PathUtil::GetFileExt(fullPath, true);
		if (Echo::StringUtil::Equal(fileExt, ".png", false))
		{
			QPixmap pixmap(fullPath);
			return QIcon(pixmap.scaled(QSize(64, 64)));
		}
		else if (Echo::StringUtil::Equal(fileExt, ".lua", false))
		{
			return QIcon(":/icon/Icon/file/lua.png");
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

	// is support this ext
	bool QPreviewHelper::isSupportExt(const Echo::String& file)
	{
		if (m_supportExts.empty())
			return true;

		Echo::String ext = Echo::PathUtil::GetFileExt(file, true);
		for (Echo::String& supportExt : m_supportExts)
		{
			if (Echo::StringUtil::Equal( supportExt, ext, false))
				return true;
		}

		return false;
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

	// item at
	QStandardItem* QPreviewHelper::itemAt(const QPoint& pos)
	{
		QModelIndex proxyIndex = m_listView->indexAt(pos);

		const QModelIndex index = m_listProxyModel->mapToSource(proxyIndex);
		return m_listModel->itemFromIndex(index);
	}

	// edit item
	void QPreviewHelper::editItem(QStandardItem* item)
	{
		if (item)
		{
			m_listView->edit(m_listProxyModel->mapFromSource(item->index()));
		}
	}

	// clicked resource
	void QPreviewHelper::onClicked(const QModelIndex& pIndex)
	{
		Echo::String resPath = m_listProxyModel ? m_listProxyModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str() : m_listModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str();
		if (Echo::PathUtil::IsDir(resPath))
		{
			Echo::PathUtil::FormatPathAbsolut(resPath, false);
		}

		emit clickedRes(resPath.c_str());
	}

	// double clicked resource
	void QPreviewHelper::onDoubleClicked(const QModelIndex& pIndex)
	{
		Echo::String resPath = m_listProxyModel ? m_listProxyModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str() : m_listModel->data(pIndex, Qt::UserRole).toString().toStdString().c_str();
		if (Echo::PathUtil::IsDir(resPath))
		{
			Echo::PathUtil::FormatPathAbsolut(resPath, false);
		}
		
		emit doubleClickedRes(resPath.c_str());
	}

	// rename res
	void QPreviewHelper::renameRes(QStandardItem* item)
	{
		if (item)
		{
			Echo::String preFilePathName = item->data(Qt::UserRole).toString().toStdString().c_str();
			Echo::String currentText = item->text().toStdString().c_str();
			if (Echo::PathUtil::IsDir(preFilePathName))
			{
				Echo::String folderName = Echo::PathUtil::GetLastDirName(preFilePathName);
				if (currentText != folderName)
				{
					Echo::String newPath = Echo::PathUtil::GetParentPath(preFilePathName) + currentText + "/";
					Echo::PathUtil::RenameDir(preFilePathName, newPath);

					item->setData(newPath.c_str(), Qt::UserRole);

					emit renamedRes( preFilePathName.c_str(), newPath.c_str());
				}
			}
			else
			{
				Echo::String folderName = Echo::PathUtil::GetFileDirPath(preFilePathName);
				Echo::String fileName = Echo::PathUtil::GetPureFilename(preFilePathName);
				if (fileName != currentText)
				{
					Echo::String newPathName = folderName + currentText;
					Echo::PathUtil::RenameFile(preFilePathName, newPathName);

					item->setData(newPathName.c_str(), Qt::UserRole);

					emit renamedRes(preFilePathName.c_str(), newPathName.c_str());
				}
			}
		}
	}
}