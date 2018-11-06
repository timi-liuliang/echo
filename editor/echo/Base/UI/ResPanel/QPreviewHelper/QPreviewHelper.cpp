#include "QPreviewHelper.h"
#include <QFileInfo>
#include <QDateTime>
#include <engine/core/util/StringUtil.h>
#include <engine/core/util/PathUtil.h>

namespace QT_UI
{
	QPreviewHelper::QPreviewHelper(QListView* view)	
		: m_listView(view)
	{
		m_listModel = new QStandardItemModel(m_listView);

		m_listProxyModel = new QSortFilterProxyModel(m_listView);
		m_listProxyModel->setSourceModel(m_listModel);
		m_listProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		m_listProxyModel->setFilterKeyColumn(0);

		m_listView->setModel(m_listProxyModel);

		QObject::connect(m_listView, SIGNAL(clicked(QModelIndex)), this, SLOT(onClicked(QModelIndex)));
		QObject::connect(m_listView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(onDoubleClicked(QModelIndex)));
		QObject::connect(m_listModel, &QStandardItemModel::itemChanged, this, &QPreviewHelper::renameRes);

		setUseIconMode();
	}

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

	void QPreviewHelper::setFilterPattern(const char* pattern)
	{
		QRegExp regExp( pattern, Qt::CaseInsensitive);
		m_listProxyModel->setFilterRegExp(regExp);
	}

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
		QStandardItem* item = nullptr;
		if (Echo::PathUtil::IsDir( filePath))
		{
			Echo::String folderName = Echo::PathUtil::GetLastDirName(filePath);
			item = new QStandardItem(QIcon(":/icon/Icon/root.png"), folderName.c_str());
		}
		else
		{
			Echo::String fileName = Echo::PathUtil::GetPureFilename(filePath, true);
			item = new QStandardItem( getFileIcon(filePath), fileName.c_str());
		}

		if (item)
		{
			item->setData(filePath, Qt::UserRole);
			item->setSizeHint(QSize( m_itemWidth, m_itemHeight));
			addToolTips(item, filePath);
			results.push_back(item);
		}
	}

	void QPreviewHelper::addToolTips(QStandardItem* item, const Echo::String& fullPath)
	{
		Echo::String fileName = Echo::PathUtil::IsDir(fullPath) ? Echo::PathUtil::GetLastDirName(fullPath) : Echo::PathUtil::GetPureFilename(fullPath);
		QFileInfo fileInfo( fullPath.c_str());
		qint64    fileSize = fileInfo.size();
		Echo::String lastModify = fileInfo.lastModified().toString("yyyy/mm/dd hh:mm:ss").toStdString().c_str();

		Echo::String tips;
		tips += "Name : " + fileName + "\n";
		tips += "Size : " + Echo::StringUtil::Format("%d kb\n", max(fileSize / 1024, 1));
		tips += "Modify : " + lastModify + "\n";
		tips += "Path : " + fullPath;
		item->setToolTip(tips.c_str());	
	}

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

	void QPreviewHelper::clear()
	{
		m_listModel->clear();
	}

	void QPreviewHelper::setUseIconMode()
	{
		m_iconSize = 64;
		m_listView->setIconSize(QSize( m_iconSize, m_iconSize));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::IconMode);
		m_listView->setMovement(QListView::Static);
		m_listView->setFlow(QListView::LeftToRight);
		m_listView->setSpacing(5);
		setItemSizeHint(68, 86);
	}

	void QPreviewHelper::setUseListMode()
	{
		m_iconSize = 30;
		m_listView->setIconSize(QSize( m_iconSize, m_iconSize));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::ListMode);
		m_listView->setMovement(QListView::Static);
		m_listView->setFlow(QListView::TopToBottom);
		m_listView->setSpacing(0);
		setItemSizeHint( 512, 30);
	}

	void QPreviewHelper::setItemSizeHint(int width, int height)
	{
		m_itemWidth = width;
		m_itemHeight = height;
		for (int i = 0; i < m_listModel->rowCount(); i++)
		{
			QStandardItem* item = m_listModel->item(i, 0);
			if (item)
				item->setSizeHint(QSize(m_itemWidth, m_itemHeight));
		}
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