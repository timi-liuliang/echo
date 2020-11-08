#include "QPropertyModel.h"
#include <vector>
#include "engine/core/util/AssertX.h"
#include <engine/core/log/Log.h>

namespace QT_UI
{
	static const int kConstraint = Qt::UserRole + 0x0010;
	std::vector<QPropertyModel*> g_qModels;

	QPropertyModel::QPropertyModel(const char* fileName, bool isEnableGB2312, QObject* parent/*= 0*/)
		: QStandardItemModel(parent)
		, m_isEnableGB2312(isEnableGB2312)
	{
		QByteArray fileContent(fileName);

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

	void  QPropertyModel::setupModelData(pugi::xml_document* cfg)
	{
		if (!cfg)
			return;

		m_cfg = cfg;

		ParseCfg();
	}

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

		// add child item
        pugi::xml_node rootItme = root.child("item");
		addChildItem(&rootItme, this->invisibleRootItem());
	}

	void  QPropertyModel::addChildItem(pugi::xml_node* firstChildElement, QStandardItem* pParentItem)
	{
		pugi::xml_node elementNode = *firstChildElement;
		while (elementNode)
		{
			QString text = m_isEnableGB2312 ? QString::fromLocal8Bit(elementNode.attribute("text").value()) : QString::fromUtf8(elementNode.attribute("text").value());
			QString toolTip = elementNode.attribute("toolTip").as_string("");

			bool bold = elementNode.attribute("bold").as_bool(false);

			int row = elementNode.attribute("row").as_int(0);
			int col = elementNode.attribute("col").as_int(0);

			// upper text
			std::string upperText = text.toStdString();
			if (!upperText.empty())
				upperText[0] = toupper(upperText[0]);

			QStandardItem* item = addChildItem(pParentItem, upperText.c_str(), bold, row, col, elementNode.attribute("widget").as_string(""), elementNode.attribute("property").as_string(""), toolTip.toStdString().c_str());
			item->setData(constraintCondition(firstChildElement), kConstraint);

			pugi::xml_node childNode = elementNode.child("item");
			if (childNode)
				addChildItem(&childNode, item);

			elementNode = elementNode.next_sibling("item");
		}
	}

	QStandardItem* QPropertyModel::addChildItem(QStandardItem* parent, const char* text, bool bold, int row, int col, const char* widget, const char* propertyName, const char* toolTip)
	{
		QString propertyNamelocal = m_isEnableGB2312 ? QString::fromLocal8Bit(propertyName) : QString::fromUtf8(propertyName);
		QStandardItem* item = new QStandardItem(text);
		QFont font = item->font(); font.setBold(bold);
		item->setToolTip(toolTip);
		item->setFont(font);
		item->setData(QString::fromUtf8(widget), Qt::UserRole);
		item->setData(propertyNamelocal, Qt::DisplayPropertyRole);
		item->setForeground(QBrush(QColor(243, 243, 243)));
		parent->setChild(row, col, item);
		m_standItems.push_back(item);

		if (strlen(propertyName))
		{
			if (m_propertyStandItmes.find(propertyNamelocal) == m_propertyStandItmes.end())
			{
				m_propertyStandItmes[propertyNamelocal] = item;
			}
			else
			{
				EchoLogError( "property name [%s] is duplicate.", propertyNamelocal.toStdString().c_str());
			}			
		}

		return item;
	}

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

	QStandardItem* QPropertyModel::findItemByText(const char* text)
	{
		foreach(QStandardItem* item, m_standItems)
		{
			if (item->text() == text)
				return item;
		}

		return NULL;
	}

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

	void QPropertyModel::setValue(const QString& propertyName, QVariant value)
	{
		if (m_valueList[propertyName] != value)
		{
			m_valueList[propertyName] = value;

			updateConstraint(propertyName, value);

			// modify value
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
}
