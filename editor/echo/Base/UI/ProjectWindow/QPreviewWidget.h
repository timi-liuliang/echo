#pragma once

#include <QWidget>
#include <QListView>

#include <sys/stat.h>
#include <vector>
#include <map>
#include <string>

QT_BEGIN_NAMESPACE
class QStandardItemModel;
class QStandardItem;
class QSortFilterProxyModel;
QT_END_NAMESPACE

namespace QT_UI
{
	/**
	 * QPreviewWidget 预览界面
	 */
	class QDragListWidget : public QListView
	{
		Q_OBJECT

	public:
		QDragListWidget(QWidget* parent = 0);
		~QDragListWidget();

		virtual void startDrag(Qt::DropActions supportedActions) override;
	};

	class QPreviewWidget : public QWidget
	{
		Q_OBJECT

		typedef std::map<std::string, std::string>  IconMap;
	public:
		// ItemInfo
		struct ItemInfo
		{
			ItemInfo()
			:m_time(0)
			{

			}
			QString		m_text;		// 文件名
			QString		m_icon;		// 图标
			__time64_t m_time;
		};

	public:
		QPreviewWidget( QWidget* parent=0);
		virtual ~QPreviewWidget();

		// 添加项
		void addItem(const char* filePath);

		// 添加项
		void addItem( const QString& text, const QString& icon);

		// 
		void createItem(const char* filePath, std::vector<QStandardItem*>& results);
		QStandardItem* createItem( const QString& text, const QString& icon);

		// 清空所有项
		void clearAllItems();

		// 设置缩略图
		void setThumbnail( const char* text, const char* texture);

		// 获取当前选中项
		QString getCurrentSelect();

		// 获取所封装的widget
		QListView* getListWidget() { return m_listView; }
		QStandardItemModel* getListModel() { return m_listModel; }

		// 设置搜寻名称
		//void setSearchName(const char* searchName) { m_searchName = searchName; }

		// 设置类型过滤
		void setTypesFilter(const std::vector<QString>& typesFilter) { m_typesFilter = typesFilter;  }

		// 设置指定后缀名对应图标
		void SetIcon(const char* ext, const char* icon){ m_iconMaps[ext] = icon; }

		// 设置图标宽高
		void setIconWH(int w, int h) { m_iconWidth = w; m_iconHeight = h; }

		// 是否需要全路径
		void isNeedFullPath(bool state) { m_isNeedFullPath = state; }

		// 添加提示
		void addToolTips(QStandardItem* item, const QString& text);

		// 添加多线程生成缩略图任务
		void addThumbnailThreadTask(const char* name, const char* thumbNailFile);

		void editItem(QStandardItem* item);

		QList<QStandardItem*> selectedItems();

		QStandardItem* itemAt(const QPoint& p);

		QStandardItem* currentItem();

		void selectItemByName(const char* const name);

		void setCurrentItem(QModelIndex index);

		void filterName(const char* const);

	public slots:
		// 单击时触发
		void onClicked(const QModelIndex& index);

		// 双击是触发
		void onDoubleClicked(const QModelIndex& index);

		// 缩略图修改
		void onThumbnailChanged(const QString& text, const QString& texture);

	public: signals:
		// 单击信号
		void Signal_onClickedItem( const QString&);

		// 双击信号
		void Signal_onDoubleClickedItem( const QString&);

		// 消息信号
		void Signal_ThumbChange( const QString&, const QString&);

		// 文本改变
		void Signal_onItemTextChanged();

	private:
		// 根据后缀名判断资源是否需要预览
		bool IsNeedPreview(const char* ext);

	protected:
		int							m_iconWidth;		// 图标宽
		int							m_iconHeight;		// 图标高
		bool						m_isNeedFullPath;	// 是否存全路径
		std::vector<ItemInfo>		m_itemInfos;		// 预览信息
		std::map<QString, QIcon*>	m_iconCaches;		// 图标缓冲

		QDragListWidget*			m_listView;			// QListWidget实现方式
		QStandardItemModel*			m_listModel;		// list model
		QSortFilterProxyModel*		m_listProxyModel;	// list sort and filter proxy model

		//WorkerThread*				m_thumbnailThread;	// 缩略图生成线程

		std::vector<QString>		m_typesFilter;		// 
		IconMap						m_iconMaps;			// 图标
	};
}