#include "QPreviewWidget.h"

#include <Engine/modules/Audio/FMODStudio/FSAudioManager.h>
#include <engine/core/util/PathUtil.h>
#include <engine/core/main/Root.h>
#include <TlHelp32.h>
#include "Studio.h"
#include <QFileInfo>
#include <QDateTime>
#include <QMimeData.h>
#include <QDrag.h>
#include <QSortFilterProxyModel>
#include <QHBoxLayout>
#include <QStandardItemModel>

namespace QT_UI
{
	/**
	 * 缩略图生成线程
	 */
	class WorkerThread
	{
	public:
		WorkerThread(QObject* parent = 0){ m_thumbExeExist = true; }

		// 设置参数
		void set( QPreviewWidget* previewWidget)
		{
			m_previeweWidget = previewWidget;
		}

		// 添加任务
		void addTask( const char* text, const char* srcTexture)
		{
			EE_LOCK_MUTEX( m_mutexTask);
			m_srcTextures.push_back( text + Echo::String( ",") + srcTexture);

			//EchoLogInfo("=========================WorkerThread::addTask:%s,%s m_srcTextures.size:%d", text, srcTexture, m_srcTextures.size());

			//resume();
		}

		// 清空任务
		void clear()
		{
			EE_LOCK_MUTEX( m_mutexTask);
			m_srcTextures.clear();
		}

		// 是否在任务队列中
		bool checkInTask(const char* text, const char* srcTexture)
		{
			EE_LOCK_MUTEX(m_mutexTask);
			std::list<Echo::String>::iterator it;
			for (it = m_srcTextures.begin(); it != m_srcTextures.end(); it++)
			{
				Echo::StringArray infos = Echo::StringUtil::Split(*it);
				if (text == infos[0].c_str() && srcTexture == infos[1].c_str())
				{
					//resume();
					return true;
				}
			}

			//resume();
			return false;
		}

		// 执行
		void processLoop()
		{
			//while (m_state==TS_Running)
			{
				if (m_srcTextures.empty() /*&& !m_thumbExeExist*/)
				{
				//	pause();
				}
				else
				{			
					if (!m_srcTextures.empty())
					{
						Echo::String srcTexture;
						Echo::String srcText;

						// 线程锁,获取下一个任务
						{
							EE_LOCK_MUTEX(m_mutexTask);
							Echo::StringArray infos = Echo::StringUtil::Split(m_srcTextures.front(), ",");
							srcText = infos[0];
							srcTexture = infos[1];		
						}

						Echo::String thumbnailPath = srcTexture;
						if (Echo::PathUtil::IsFileExist(thumbnailPath))
						{
							m_previeweWidget->setThumbnail(srcText.c_str(), thumbnailPath.c_str());
							EE_LOCK_MUTEX(m_mutexTask);
							m_srcTextures.pop_front();
						}
					}
				}
				
				// 线程sleep
				Echo::ThreadSleepByMilliSecond(50);
			}
		}

		// 获取进程数量
		int getProcessCount(const TCHAR* szExeName)
		{
			TCHAR sztarget[MAX_PATH];
			lstrcpy(sztarget, szExeName);
			CharLowerBuff(sztarget, MAX_PATH);

			int count = 0;
			PROCESSENTRY32 my;
			HANDLE l = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (((int)l) != -1)
			{
				my.dwSize = sizeof(my);
				if (Process32First(l, &my))
				{
					do 
					{
						CharLowerBuff(my.szExeFile, MAX_PATH);
						if (lstrcmp(sztarget, my.szExeFile) == 0)
						{
							count++;
						}
					} while (Process32Next(l, &my));
				}
				CloseHandle(l);
			}
			return count;
		}

	protected:
		QPreviewWidget*		   m_previeweWidget;	// 预览窗口
		std::list<Echo::String>m_srcTextures;		// 源纹理
		EE_MUTEX(				m_mutexTask);		// 请求队列互斥量
		bool					m_thumbExeExist;	// 缩略图进程是否存在
	};

	QDragListWidget::QDragListWidget(QWidget* parent)
		: QListView(parent)
	{
		setDragEnabled(true);
		setDragDropMode(QAbstractItemView::DragOnly);
	}

	QDragListWidget::~QDragListWidget()
	{

	}

	void QDragListWidget::startDrag(Qt::DropActions supportedActions)
	{
		if (NULL == model() || NULL == selectionModel())
			return;

		QModelIndexList items = selectionModel()->selectedIndexes();
		int count = items.count();

		//QList<QListWidgetItem*> items = selectedItems();
		//int count = items.count();
		
		QMimeData* mimeData = new QMimeData;
		if (count == 1)
		{
			mimeData->setText(model()->data(items[0], Qt::UserRole).toString());
			mimeData->setProperty("TYPE", QVariant("SINGLE"));
		}
		else
		{
			QString totalNames;
			for (int i = 0; i < count; i++)
			{
				//QListWidgetItem* item = items.at(i);
				totalNames += model()->data(items[i], Qt::UserRole).toString();
				totalNames += QString(";");
			}
			mimeData->setText(totalNames);
			mimeData->setProperty("TYPE", QVariant("MULTIPLE"));
		}	
		QDrag* drag = new QDrag(this);
		drag->setMimeData(mimeData);
		drag->exec(Qt::MoveAction);
	}

	// 构造函数
	QPreviewWidget::QPreviewWidget( QWidget* parent)
		: QWidget(parent)
		, m_iconWidth(120)
		, m_iconHeight(150)
		, m_isNeedFullPath(false)
		, m_listView(NULL)
		, m_listModel(NULL)
		, m_listProxyModel(NULL)
	{
		// 初始化ListWidget
		m_listView = new QDragListWidget(this);
		m_listView->setIconSize(QSize(120, 120));
		m_listView->setResizeMode(QListView::Adjust);
		m_listView->setViewMode(QListView::IconMode);
		m_listView->setMovement(QListView::Static);
		m_listView->setSpacing(10);

		// list model
		m_listModel = new QStandardItemModel(m_listView);

		// list sort and filter proxy model
		m_listProxyModel = new QSortFilterProxyModel(m_listView);
		m_listProxyModel->setSourceModel(m_listModel);
		m_listProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
		m_listProxyModel->setFilterKeyColumn(0);

		m_listView->setModel(m_listProxyModel);

		// 设置图标
		SetIcon("lightarray", ":/icon/Icon/lightarray.png");
		SetIcon("scenario", ":/icon/Icon/SpriteAnimation.png");
		SetIcon("project", ":/icon/Icon/project.png");
		SetIcon("h", ":/icon/Icon/cplusplus_header.png");
		SetIcon("cpp", ":/icon/Icon/cplusplus_cpp.png");
		SetIcon("lua", ":/icon/Icon/lua_icon.png");

		// 添加布局
		QHBoxLayout* hLayout = new QHBoxLayout(this);
		hLayout->setSpacing(0);
		hLayout->setContentsMargins(0, 0, 0, 0);
		this->setLayout(hLayout);
		this->layout()->addWidget(m_listView);

		// 开启缩略图线程
		//m_thumbnailThread = EchoNew( WorkerThread(this));
		//m_thumbnailThread->set(this);
		//m_thumbnailThread->start();

		// 消息链接
		QObject::connect(m_listView, &QListView::clicked, this, &QPreviewWidget::onClicked);
		QObject::connect(m_listView, &QListView::doubleClicked, this, &QPreviewWidget::onDoubleClicked);
		QObject::connect(this, SIGNAL(Signal_ThumbChange(const QString&, const QString&)), this, SLOT(onThumbnailChanged(const QString&, const QString&)));

		m_listView->setDragEnabled(true);
		m_listView->setDragDropMode(QAbstractItemView::DragOnly);
		m_listView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	}

	// 析构函数
	QPreviewWidget::~QPreviewWidget()
	{
		//m_thumbnailThread->shutdown();
		//EchoSafeDelete(m_thumbnailThread, WorkerThread);

		for (auto& it : m_iconCaches)
		{
			delete it.second;
		}
	}

	void QPreviewWidget::addItem(const char* filePath)
	{
		std::vector<QStandardItem*> results;
		createItem(filePath, results);
		for (std::vector<QStandardItem*>::const_iterator it = results.begin(); it != results.end(); ++it)
			m_listModel->appendRow(*it);
	}

	// 添加项
	void QPreviewWidget::createItem(const char* filePath, std::vector<QStandardItem*>& results)
	{
		QStandardItem* newItem = NULL;
		Echo::String previewFile = filePath;
		Echo::StringUtil::LowerCase(previewFile);
		if (Echo::PathUtil::IsDir(previewFile))
			return;

		Echo::String fileName = Echo::PathUtil::GetPureFilename(previewFile);
		Echo::String fileExt = Echo::PathUtil::GetFileExt(fileName);
		Echo::String filePreTex;

		// 是否需要预览
		if (!IsNeedPreview(fileExt.c_str()))
			return;
		if (fileExt == "bank")
		{
			// 获取所有声音事件
			//if (EchoAudioManager)
			{
				Echo::StringArray bankEvents;
				Echo::FSAudioManager::instance()->getAudioEvents(bankEvents, fileName.c_str());
				for (size_t i = 0; i < bankEvents.size(); i++)
				{
					//if (!m_searchName.isEmpty())
					//{
					//	if (QString::fromStdString(bankEvents[i]).contains(m_searchName))
					//	{
					//		addItem(m_isNeedFullPath ? filePath : bankEvents[i].c_str(), ":/icon/Icon/AudioSource.png");
					//	}
					//}
					//else
					{
						newItem = createItem(m_isNeedFullPath ? filePath : bankEvents[i].c_str(), ":/icon/Icon/AudioSource.png");
						results.push_back(newItem);
					}
				}
			}
		}
		else
		{
			Echo::String thumbNailFile = Studio::AStudio::instance()->getThumbnailPath(fileName);
			Echo::String fileExt = Echo::PathUtil::GetFileExt(fileName, false);

			IconMap::iterator it = m_iconMaps.find(fileExt);
			if (QFile::exists(QString(thumbNailFile.c_str())))
				filePreTex = thumbNailFile.c_str();
			else if (it != m_iconMaps.end())
				filePreTex = it->second;
			else
				filePreTex = ":///icon/Icon/xml.png";

			//if (!m_searchName.isEmpty())
			//{
			//	if (QString::fromStdString(fileName).contains(m_searchName))
			//	{
			//		addItem(filePath, filePreTex.c_str());
			//	}
			//}
			//else
			{
				newItem = createItem(filePath, filePreTex.c_str());
				results.push_back(newItem);
			}
		}
		//return newItem;
	}

	void QPreviewWidget::addItem( const QString& text, const QString& icon)
	{
		QStandardItem* newItem = createItem(text, icon);
		m_listModel->appendRow(newItem);
	}

	// 添加项
	QStandardItem* QPreviewWidget::createItem( const QString& text, const QString& icon)
	{
		QStandardItem* newItem = NULL;
		ItemInfo iteminfo;
		iteminfo.m_text = text;
		iteminfo.m_icon = icon;
		m_itemInfos.push_back( iteminfo);

		Echo::String fileNameWithoutExt = Echo::PathUtil::GetPureFilename(text.toStdString().c_str(), false);
		Echo::String fileName = Echo::PathUtil::GetPureFilename(text.toStdString().c_str());
		Echo::String ext = Echo::PathUtil::GetFileExt(text.toStdString().c_str(), false);
		bool isFmodEvent = Echo::StringUtil::StartWith(text.toStdString().c_str(), "event");

		//QListWidgetItem* widgetItem = NULL;
		QStandardItem* widgetItem = NULL;

		// 生成缩略图
		Echo::String thumbnailPath;
		if( !icon.isEmpty() && icon[0] == ':')
		{
			static std::map<QString, QIcon*> sicon;
			if( sicon.find( icon)==sicon.end())
			{
				QPixmap pixmap( icon);
				sicon[icon] = new QIcon( pixmap.scaled( QSize( 120, 120)));
			}
				
			//widgetItem = new QListWidgetItem(*sicon[icon], fileNameWithoutExt.c_str());
			widgetItem = new QStandardItem(*sicon[icon], fileNameWithoutExt.c_str());
			
		}
		else if (!icon.isEmpty() && ext == "echo")
		{
			QPixmap pixmap(icon);
			QIcon qicon(pixmap.scaled(QSize(120, 120)));
			widgetItem = new QStandardItem( qicon, fileNameWithoutExt.c_str());
		}
		else
		{
			//m_thumbnailThread->addTask(fileName.c_str(), icon.toStdString().c_str());
			thumbnailPath = "";

			//widgetItem = new QListWidgetItem(QIcon(thumbnailPath.c_str()), fileNameWithoutExt.c_str());
			widgetItem = new QStandardItem(QIcon(thumbnailPath.c_str()), fileNameWithoutExt.c_str());
		}
		addToolTips(widgetItem, text);

		// 设置属性
		if (widgetItem)
		{
			widgetItem->setSizeHint(QSize( m_iconWidth, m_iconHeight));
			widgetItem->setFlags(widgetItem->flags() | Qt::ItemIsEditable);
			widgetItem->setData(isFmodEvent ? text : (m_isNeedFullPath ? text : fileName.c_str()), Qt::UserRole);
			//m_listWidget->addItem(widgetItem);
			//m_listModel->appendRow(widgetItem);
		}
		return widgetItem;
	}

	void QPreviewWidget::addThumbnailThreadTask(const char* name, const char* thumbNailFile)
	{
		//m_thumbnailThread->addTask(name, thumbNailFile);
	}

	// 获取当前选中项
	QString QPreviewWidget::getCurrentSelect()
	{
		//QListWidgetItem* item = m_listWidget->currentItem();

		//return item ? item->data(Qt::UserRole).toString() : "";

		const QModelIndex proxyIndex = m_listView->currentIndex();
		const QModelIndex currentIndex = m_listProxyModel->mapToSource(proxyIndex);
		return m_listModel->data(currentIndex, Qt::UserRole).toString();
	}

	void QPreviewWidget::addToolTips(QStandardItem* item, const QString& text)
	{
		Echo::String fileName = text.toStdString().c_str();
		fileName = Echo::PathUtil::GetPureFilename(fileName);
		Echo::String fileFullName = "";
		if (Echo::PathUtil::IsAbsolutePath(text.toStdString().c_str()))
		{
			fileFullName = text.toStdString().c_str();
		}
		else
		{
			//fileFullName = EchoResourceManager->getFileLocation(fileName);
		}

		QFileInfo inputFileInfo(fileFullName.c_str());
		qint64 fileSize = inputFileInfo.size();
		fileSize = max((fileSize / 1024), 1);
		QDateTime time = inputFileInfo.lastModified();
		QString lastModify = time.toString("yyyy/MM/dd hh:mm:ss");
		Echo::String fileInfos;
		fileInfos = Echo::StringUtil::Format("名称：%s\n大小：%ldKB\n修改时间：", \
			fileName.c_str(), fileSize);

		QString qtFileInfos = QString::fromLocal8Bit(fileInfos.c_str());
		qtFileInfos.append(lastModify);
		Echo::String fullPath = "\n路径：" + fileFullName;
		qtFileInfos.append(QString::fromLocal8Bit(fullPath.c_str()));

		item->setToolTip(qtFileInfos);
	}

	// 设置缩略图
	void QPreviewWidget::setThumbnail( const char* text, const char* texture)
	{
		emit Signal_ThumbChange( text, texture);
	}

	// 缩略图修改
	void QPreviewWidget::onThumbnailChanged(const QString& text, const QString& texture)
	{
		for ( int i = 0; i < m_listModel->rowCount(); i++ )
		{
			//QListWidgetItem* item = m_listWidget->item(i);
			QModelIndex itemIndex = m_listModel->index(i, 0);
			QString fileName = m_listModel->data(itemIndex, Qt::UserRole).toString();
			if ( fileName == text )
			{
				auto icon = m_iconCaches.find(texture);
				if ( icon == m_iconCaches.end() )
				{
					m_iconCaches[texture] = new QIcon(texture);
					icon = m_iconCaches.find(texture);
				}

				QStandardItem* item = m_listModel->itemFromIndex(itemIndex);
				item->setIcon(*(icon->second));
			}
		}
	}

	// 清空所有项
	void QPreviewWidget::clearAllItems()
	{
		m_itemInfos.clear();
		//m_listWidget->clear();
		m_listModel->clear();

		//m_thumbnailThread->clear();
	}

	// 单击时触发
	void QPreviewWidget::onClicked(const QModelIndex& index)
	{
		//if( item)
		{
			QString text = index.data(Qt::UserRole).toString();
			emit Signal_onClickedItem(text);
		}
	}

	// 双击是触发
	void QPreviewWidget::onDoubleClicked(const QModelIndex& index)
	{
		//if( item)
		{
			QString text = index.data(Qt::UserRole).toString();
			emit Signal_onDoubleClickedItem(text);
		}
	}

	// 根据后缀名判断资源是否需要预览
	bool QPreviewWidget::IsNeedPreview(const char* ext)
	{
		if (!m_typesFilter.size())
			return true;

		for (size_t i = 0; i < m_typesFilter.size(); i++)
		{
			if (ext == m_typesFilter[i])
				return true;
		}

		return false;
	}

	void QPreviewWidget::editItem(QStandardItem* item)
	{
		if (NULL == item)
			return;
		m_listView->edit(item->index());
	}

	QList<QStandardItem*> QPreviewWidget::selectedItems()
	{
		QModelIndexList indices = m_listView->selectionModel()->selectedIndexes();
		QList<QStandardItem*> result;
		QModelIndexList::iterator it = indices.begin();
		for (; it != indices.end(); ++it)
		{
			const QModelIndex index = m_listProxyModel->mapToSource(*it);
			result.push_back(m_listModel->itemFromIndex(index));
		}
		return result;
	}

	QStandardItem* QPreviewWidget::itemAt(const QPoint& p)
	{
		QModelIndex proxyIndex = m_listView->indexAt(p);
		const QModelIndex index = m_listProxyModel->mapToSource(proxyIndex);
		return m_listModel->itemFromIndex(index);
	}

	QStandardItem* QPreviewWidget::currentItem()
	{
		QModelIndex proxyIndex = m_listView->currentIndex();
		const QModelIndex index = m_listProxyModel->mapToSource(proxyIndex);
		return m_listModel->itemFromIndex(index);
	}

	void QPreviewWidget::selectItemByName(const char* const name)
	{
		if (NULL == m_listView || NULL == m_listModel)
			return;
		if (m_listModel->rowCount() < 1)
			return;

		//QListWidget* previewWidget = m_previewerWidget->getListWidget();
		//if (previewWidget && previewWidget->count() >= 1)
		for (int i = 0; i < m_listModel->rowCount(); ++i)
		{
			//QListWidgetItem* item = m_listWidget->item(i);
			QModelIndex itemIndex = m_listModel->index(i, 0);
			Echo::String itemData = m_listModel->data(itemIndex, Qt::UserRole).toString().toStdString().c_str();
			if (itemData == name)
			{
				setCurrentItem(itemIndex);
				break;
			}
		}

		{
			//QListWidgetItem* selectItem = previewWidget->item(previewWidget->count() - 1);

			//for (int i = 0; i < previewWidget->count(); ++i)
			//{
			//	QListWidgetItem* widgetItem = previewWidget->item(i);
			//	Echo::String itemData = widgetItem->data(Qt::UserRole).toString().toStdString().c_str();
			//}

		}
	}

	void QPreviewWidget::setCurrentItem(QModelIndex index)
	{
		QItemSelectionModel::SelectionFlag flag = QItemSelectionModel::NoUpdate;
		switch (m_listView->selectionMode())
		{
		case QAbstractItemView::SingleSelection	: flag = QItemSelectionModel::ClearAndSelect;	break;
		case QAbstractItemView::NoSelection		: flag = QItemSelectionModel::NoUpdate;			break;
		default									: flag = QItemSelectionModel::SelectCurrent;	break;
		};
		m_listView->selectionModel()->setCurrentIndex(index, flag);
	}

	void QPreviewWidget::filterName(const char* const s)
	{
		QRegExp regExp(s, Qt::CaseInsensitive);
		m_listProxyModel->setFilterRegExp(regExp);
	}



}