#pragma once

#include <map>
#include <functional>
#include <QStandardItem>
#include <QStandardItemModel>
#include <thirdparty/pugixml/pugixml.hpp>

using namespace std;

namespace QT_UI
{
	typedef std::function<void(const char*)> SubEditCb; 

	//------------------------------------------
	// 蔘PropertyModel 2010-03-15  captain
	//------------------------------------------
	class QPropertyModel : public QStandardItemModel
	{
		Q_OBJECT

		typedef map<QString, QVariant> AttribList;
	public:
		// 构造函数
		QPropertyModel(const char* fileName, bool isEnableGB2312 = false, QObject* parent = 0);
		~QPropertyModel();

		// 设置ID
		void setID(const QString& id) { m_id = id; }

		// 获取ID
		const QString& getID() { return m_id; }

		// 设置数据
		void  setupModelData(pugi::xml_document* cfg);

		// 根据属性查找数据
		bool findValue(QVariant& value, const QString& propertyName);

		// 设置数据
		void setValue(const QString& propertyName, QVariant value);

		// 设置数据
		static bool setValue(const QString& modelName, const QString& propertyName, QVariant value);

		// 根据text查找item
		QStandardItem* findItemByText(const char* text);

		// 添加子结点
		QStandardItem* addChildItem(QStandardItem* parent, const char* text, bool bold, int row, int col, const char* widget, const char* property);

		// 除除子结点
		void removeChildItem(QStandardItem* parent);

		// 抛出自定义Signal
		void ThrowSelfDefineSig(QString sigName, QString propertyName, QVariant value);

		// 设置回调;
		void set2ndSubEditorCallback(SubEditCb cb);

		// 触发回调;
		void on2ndSubEditorCallback(const char* fileName); 

	private:
		// 解析配置文件
		void ParseCfg();

		// 递归添加
		void addChildItem(pugi::xml_node* pFirstChildElement, QStandardItem* pParentItem);

		// 更新约束
		void updateConstraint(const QString& propertyName, QVariant value);

		// 读取约束
		QMap<QString, QVariant> constraintCondition(pugi::xml_node* itemNode);

	signals:
		// 数据变动
		void Signal_ValueChanged(const QString& propertyName, QVariant value);
		void Signal_ValueChanged(const QString& propertyName, QVariant value, int row, int column);

		//抛出自定义的信号,主要是为了解决一些属性修改弹窗涉及到过多的引擎逻辑,所以为了解耦,抛给引擎逻辑处理
		void Signal_ThrowSelfDefineSig(const QString& singalStr, const QString& propertyNamej,QVariant value);

	private:
		QString						m_id;					// 标识
		pugi::xml_document*			m_cfg;
		AttribList					m_valueList;
		vector<QStandardItem*>		m_standItems;			// 表项
		map<QString, QStandardItem*>m_propertyStandItmes;	// 属性对应的StandItem
		bool						m_isEnableGB2312;

		SubEditCb					m_2ndSubEditorCallback; 
	};
}
