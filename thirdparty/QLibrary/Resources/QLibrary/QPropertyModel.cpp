#include "QPropertyModel.h"
#include <vector>
#include "engine/core/Util/AssertX.h"

namespace QT_UI
{
	static const int kConstraint = Qt::UserRole + 0x0010;
	std::vector<QPropertyModel*> g_qModels;

	// 构造函数
	QPropertyModel::QPropertyModel(const char* fileName, bool isEnableGB2312, QObject* parent/*= 0*/)
		: QStandardItemModel(parent)
		, m_isEnableGB2312(isEnableGB2312)
	{
		QByteArray fileContent(fileName);

		// 创建Item
		m_cfg = new pugi::xml_document;
		m_cfg->load(fileContent.data());

		ParseCfg();

		g_qModels.push_back(this);
	}

	QPropertyModel::~QPropertyModel()
	{
		delete m_cfg;
		m_cfg = NULL;

		g_qModels.erase( remove(g_qModels.begin(), g_qModels.end(), this), g_qModels.end());
	}

	// 设置数据
	void  QPropertyModel::setupModelData(pugi::xml_document* cfg)
	{
		if (!cfg)
			return;

		m_cfg = cfg;

		ParseCfg();
	}

	// 解析配置文件
	void  QPropertyModel::ParseCfg()
	{
		clear();

		// headerLabel
		pugi::xml_node root = m_cfg->child("cfg");
		pugi::xml_node headers = root.child("headers");
		if (headers)
		{
			QStringList headerLables;
			pugi::xml_node header = headers.child("header");
			while (header)
			{
				headerLables << header.attribute("text").as_string();
				header = header.next_sibling("header");
			}
			setHorizontalHeaderLabels(headerLables);
		}

		// 表项
		addChildItem(&root.child("item"), this->invisibleRootItem());
	}

	// 递归添加
	void  QPropertyModel::addChildItem(pugi::xml_node* pFirstChildElement, QStandardItem* pParentItem)
	{
		while (pFirstChildElement)
		{
			QString text = m_isEnableGB2312 ? QString::fromLocal8Bit(pFirstChildElement->attribute("text").value())
				: QString::fromUtf8(pFirstChildElement->attribute("text").value());

			bool bold = pFirstChildElement->attribute("bold").as_bool(false);

			int row = pFirstChildElement->attribute("row").as_int(0);
			int col = pFirstChildElement->attribute("col").as_int(0);

			QStandardItem* item = addChildItem(pParentItem, text.toStdString().c_str(), bold, row, col, pFirstChildElement->attribute("widget").as_string(""), pFirstChildElement->attribute("property").as_string(""));
			item->setData(constraintCondition(pFirstChildElement), kConstraint);
			addChildItem(&pFirstChildElement->child("item"), item);

			pFirstChildElement = &pFirstChildElement->next_sibling("item");
		}
	}

	// 添加子结点
	QStandardItem* QPropertyModel::addChildItem(QStandardItem* parent, const char* text, bool bold, int row, int col, const char* widget, const char* propertyName)
	{

		QString propertyNamelocal = m_isEnableGB2312 ? QString::fromLocal8Bit(propertyName) : QString::fromUtf8(propertyName);
		QStandardItem* item = new QStandardItem(text);
		QFont font = item->font(); font.setBold(bold);
		item->setFont(font);
		item->setData(QString::fromUtf8(widget), Qt::UserRole);
		item->setData(propertyNamelocal, Qt::DisplayPropertyRole);
		parent->setChild(row, col, item);
		m_standItems.push_back(item);

		if (strlen(propertyName))
		{
			EchoAssertX(m_propertyStandItmes.find(propertyNamelocal) == m_propertyStandItmes.end(),
				"property name [%s] is duplicate.", propertyNamelocal.toStdString().c_str());
			m_propertyStandItmes[propertyNamelocal] = item;
		}

		return item;
	}

	// 除除子结点
	void QPropertyModel::removeChildItem(QStandardItem* parent)
	{
		for (vector<QStandardItem*>::iterator it = m_standItems.begin(); it != m_standItems.end();)
		{
			if ((*it)->parent() == parent)
				it = m_standItems.erase(it);
			else
				it++;
		}

		for (int i = 0; i < parent->columnCount(); i++)
			parent->removeColumn(i);
	}

	// 根据text查找item
	QStandardItem* QPropertyModel::findItemByText(const char* text)
	{
		foreach(QStandardItem* item, m_standItems)
		{
			QString xx = item->text();
			if (item->text() == text)
				return item;
		}

		return NULL;
	}

	// 根据属性查找数据
	bool QPropertyModel::findValue(QVariant& value, const QString& propertyName)
	{
		AttribList::iterator it = m_valueList.find(propertyName);
		if (it != m_valueList.end())
		{
			value = it->second;

			return true;
		}

		return false;
	}

	// 设置数据
	void QPropertyModel::setValue(const QString& propertyName, QVariant value)
	{
		// 正则表达式验证
		//if( value.mmdfkdf)
		//	return;
		if (m_valueList[propertyName] != value)
		{
			m_valueList[propertyName] = value;

			updateConstraint(propertyName, value);

			// 修改值
			map<QString, QStandardItem*>::iterator it = m_propertyStandItmes.find(propertyName);
			if (it != m_propertyStandItmes.end())
			{
				QStandardItem* item = it->second;
				if (item)
				{
					item->setText(value.toString());
					emit Signal_ValueChanged(propertyName, value);
					emit Signal_ValueChanged(propertyName, value, item->row(), item->column());
				}
			}
		}
	}

	// 设置数据
	bool QPropertyModel::setValue(const QString& modelName, const QString& propertyName, QVariant value)
	{
		for (QPropertyModel* model : g_qModels)
		{
			if (model && !modelName.isEmpty() && model->getID() == modelName)
			{
				model->setValue(propertyName, value);
				return true;
			}
		}

		return false;
	}

	//TODO:Property约束报错
	void QPropertyModel::updateConstraint(const QString& propertyName, QVariant value)
	{
		//std::for_each(std::begin(m_standItems), std::end(m_standItems), [&](QStandardItem* item) {
			//auto constraint = item->data(kConstraint).toMap();
			//auto it = constraint.find(propertyName);
			//if (it != constraint.end())
			//{
			//	auto list = it.value().toStringList()
			//	int i = list.count(value.toString());
			//	item->setEnabled(i != 0);
			//}
		//});
	}

	QMap<QString, QVariant> QPropertyModel::constraintCondition(pugi::xml_node* itemNode)
	{
		QMap<QString, QVariant> constraint;
		pugi::xml_node node = itemNode->child("constraint");
		if (node)
		{
			auto attr = node.first_attribute();
			while (attr)
			{
				auto value = QString(attr.value()).split(",");
				constraint.insert(QString(attr.name()), value);
				attr = attr.next_attribute();
			}
		}
		return constraint;
	}

	void QPropertyModel::ThrowSelfDefineSig(QString sigName, QString propertyName, QVariant value)
	{
		emit Signal_ThrowSelfDefineSig(sigName,propertyName,value);
	}

	void QPropertyModel::set2ndSubEditorCallback(SubEditCb cb)
	{
		m_2ndSubEditorCallback = cb; 
	}

	void QPropertyModel::on2ndSubEditorCallback(const char* fileName)
	{
		if (!m_2ndSubEditorCallback)
		{
			// EchoLogError("[QPropertyModel::on2ndSubEditorCallback:%d]::No Callback Setted.", __LINE__); 
			return; 
		}

		m_2ndSubEditorCallback(fileName);
	}
}
