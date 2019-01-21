#include <algorithm>
#include "QDirectoryModel.h"
#include <engine/core/util/PathUtil.h>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QDirectoryModel::QDirectoryModel()
		: QStandardItemModel()
	{
	}

	void QDirectoryModel::SetRootPath(const char* rootPath, const char* extFilter, QTreeView* treeView, QSortFilterProxyModel* proxy)
	{
		updateRootPath(rootPath);

		Echo::StringArray exts = Echo::StringUtil::Split(extFilter, "|");
		for (size_t i = 0; i<exts.size(); i++)
			m_exts.push_back(exts[i].c_str());

		m_treeView = treeView;
		m_proxy = proxy;
		connect(m_treeView, SIGNAL(expanded(const QModelIndex&)), this, SLOT(OnExpandedFilter(const QModelIndex&)));
		connect(m_treeView, SIGNAL(collapsed(const QModelIndex&)), this, SLOT(OnExpandedFilter(const QModelIndex&)));
		connect(m_treeView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnSelectedFile(const QModelIndex&)));
		connect(m_treeView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(OnEditFile(const QModelIndex&)));
	}

	void QDirectoryModel::updateRootPath(const char* rootPath)
	{
		m_rootPath = rootPath;
		Echo::PathUtil::FormatPathAbsolut(m_rootPath, false);
	}

	bool QDirectoryModel::IsSupportExt(const string& ext)
	{
		if (!m_exts.empty())
		{
			if (std::find(m_exts.begin(), m_exts.end(), ext) == m_exts.end())
				return false;
		}

		return true;
	}

	void QDirectoryModel::Clean()
	{
		this->clear();
		m_dirItems.clear();
		m_activeItem = nullptr;
	}

	void QDirectoryModel::setCurrentSelect(const char* dir)
	{
		// 查找Item
		QStandardItem* lastItem = NULL;
		for (size_t i = 0; i<m_dirItems.size(); i++)
		{
			QStandardItem* item = m_dirItems[i];
			if (item && item->data(Qt::UserRole).toString() == dir)
			{
				lastItem = item;
				break;
			}
		}

		if (lastItem)
		{
			// 设置当前选中
			QModelIndex lastIdx = indexFromItem(lastItem);
			m_treeView->setCurrentIndex(lastIdx);
			OnSelectedFile(lastIdx);

			// 展开父
			QStandardItem* parentItem = lastItem->parent();
			while (parentItem)
			{
				QModelIndex parentIdx = indexFromItem(parentItem);
				m_treeView->expand(parentIdx);

				parentItem = parentItem->parent();
			}

			// 展开自身
			m_treeView->expand(lastIdx);
		}
	}

	void QDirectoryModel::Refresh(bool volatile* interrupt)
	{
		if (Echo::PathUtil::IsDirExist(m_rootPath.c_str()))
		{
			if (NULL != interrupt && *interrupt)
				return;

			Echo::String pathName = m_rootPath.c_str();
			Echo::PathUtil::FormatPath(pathName);
			Echo::String dirName = Echo::PathUtil::GetLastDirName(pathName);
			QStandardItem* rootItem = new QStandardItem;
			rootItem->setText("Res://");
			rootItem->setIcon(m_iconMaps["root"]);
			rootItem->setData(m_rootPath.c_str(), Qt::UserRole);

			invisibleRootItem()->setChild(invisibleRootItem()->rowCount(), 0, rootItem);

			m_dirItems.push_back(rootItem);
			RecursiveDir(m_rootPath, rootItem, interrupt);

			// expand root index
			m_treeView->expand(rootItem->index());
		}
	}

	QIcon QDirectoryModel::getFileIcon(const char* fullPath)
	{
		Echo::String fileExt = Echo::PathUtil::GetFileExt(fullPath, true);
		if (fileExt == ".png")
		{
			QPixmap pixmap(fullPath);
			return QIcon(pixmap.scaled(QSize(64, 64)));
		}

		return m_iconMaps[fileExt.c_str()];
	}

	void QDirectoryModel::RecursiveDir(const string& dir, QStandardItem* parentItem, volatile bool* interrupt)
	{
		vector<QStandardItem*> dirItems;
		vector<QStandardItem*> fileItems;

		Echo::StringArray fileDirs;
		Echo::PathUtil::EnumFilesInDir(fileDirs, dir.c_str(), true, false, true);
		for (size_t i = 0; i<fileDirs.size(); i++)
		{
			if (NULL != interrupt && *interrupt)
				return;
			if (Echo::PathUtil::IsDir(fileDirs[i]))
			{
				Echo::PathUtil::FormatPathAbsolut(fileDirs[i], false);
				Echo::String dirName = Echo::PathUtil::GetLastDirName(fileDirs[i]);
				QStandardItem* childItem = new QStandardItem;
				childItem->setText(dirName.c_str());
				childItem->setIcon(m_iconMaps["filter"]);
				childItem->setData(fileDirs[i].c_str(), Qt::UserRole);

				dirItems.push_back(childItem);

				m_dirItems.push_back(childItem);
				RecursiveDir(fileDirs[i].c_str(), childItem, interrupt);
			}
			else
			{
				Echo::String fileExt = Echo::PathUtil::GetFileExt(fileDirs[i], true);
				if (IsSupportExt(fileExt))
				{
					Echo::PathUtil::FormatPathAbsolut(fileDirs[i], false);
					Echo::String pureFileName = Echo::PathUtil::GetPureFilename(fileDirs[i]);
					QStandardItem* childItem = new QStandardItem;
					childItem->setText(pureFileName.c_str());
					childItem->setIcon(getFileIcon(fileDirs[i].c_str()));
					childItem->setData(fileDirs[i].c_str(), Qt::UserRole);

					fileItems.push_back(childItem);
				}
			}
		}

		int tRow = 0;

		// 先插入目录
		{
			for (size_t i = 0; i < dirItems.size(); i++)
			{
				parentItem->setChild(tRow, 0, dirItems[i]);
				tRow++;
			}

			// 插入文件
			for (size_t i = 0; i < fileItems.size(); i++)
			{
				parentItem->setChild(tRow, 0, fileItems[i]);
				tRow++;
			}
		}
	}

	void QDirectoryModel::OnExpandedFilter(const QModelIndex& pIndex)
	{
		/*if( m_treeView->isExpanded( pIndex))
		itemFromIndex( pIndex)->setIcon( m_iconMaps["filterexpend"]);
		else
		itemFromIndex( pIndex)->setIcon( m_iconMaps["filter"]);*/
	}

	QString QDirectoryModel::getFileUnderMousePos(const QPoint& pos)
	{
		QModelIndex index = m_treeView->indexAt(pos);
		QString filePath = m_proxy ? m_proxy->data(index, Qt::UserRole).toString() : this->data(index, Qt::UserRole).toString();

		return filePath;
	}

	void QDirectoryModel::OnSelectedFile(const QModelIndex& pIndex)
	{
        //Echo::Dword currenTime = GetTickCount();
		//Echo::Dword elapsedTime = currenTime - m_selectTime;
        m_selectTime = 0;//currenTime;

		if (m_currentSelect == pIndex)
			return;

		m_currentSelect = pIndex;
		QString filePath = m_proxy ? m_proxy->data(pIndex, Qt::UserRole).toString() : this->data(pIndex, Qt::UserRole).toString();

		// 取消激活 // 粗体显示
		if (m_activeItem)
			m_activeItem->setFont(m_treeView->font());

		// 设置激活
		m_activeItem = itemFromIndex(pIndex);
		if (m_activeItem)
		{
			// 界面响应
			QFont font = m_treeView->font();
			font.setBold(true);

			m_activeItem->setFont(font);
		}

		emit FileSelected(filePath.toStdString().c_str());
	}

	void QDirectoryModel::OnEditFile(const QModelIndex& pIndex)
	{
		QString filePath = m_proxy ? m_proxy->data(pIndex, Qt::UserRole).toString() : this->data(pIndex, Qt::UserRole).toString();

		// 取消激活 // 粗体显示
		if (m_activeItem)
			m_activeItem->setFont(m_treeView->font());

		// 设置激活
		m_activeItem = itemFromIndex(pIndex);
		if (m_activeItem)
		{
			// 界面响应
			QFont font = m_treeView->font();
			font.setBold(true);

			m_activeItem->setFont(font);
		}

		emit FileEdit(filePath.toStdString().c_str());
	}
}
