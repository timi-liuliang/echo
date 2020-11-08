#pragma once

#include <map>
#include <functional>
#include <QStandardItem>
#include <QStandardItemModel>
#include <thirdparty/pugixml/pugixml.hpp>

using namespace std;

namespace QT_UI
{
	/**
	 * PropertyModel 2010-03-15  captain
	 */
	class QPropertyModel : public QStandardItemModel
	{
		Q_OBJECT

		typedef map<QString, QVariant> AttribList;
	public:
		QPropertyModel(const char* fileName, bool isEnableGB2312 = false, QObject* parent = 0);
		~QPropertyModel();

		// id
		void setID(const QString& id) { m_id = id; }
		const QString& getID() { return m_id; }

		// set data
		void  setupModelData(pugi::xml_document* cfg);

		// find value
		bool findValue(QVariant& value, const QString& propertyName);

		// set value
		void setValue(const QString& propertyName, QVariant value);
		static bool setValue(const QString& modelName, const QString& propertyName, QVariant value);

		// find by text
		QStandardItem* findItemByText(const char* text);

		// child operates
		QStandardItem* addChildItem(QStandardItem* parent, const char* text, bool bold, int row, int col, const char* widget, const char* property, const char* toolTip);
		void removeChildItem(QStandardItem* parent);

	private:
		// parse config
		void ParseCfg();

		// add child item
		void addChildItem(pugi::xml_node* pFirstChildElement, QStandardItem* pParentItem);

		// update constraint
		void updateConstraint(const QString& propertyName, QVariant value);

		// read constraint
		QMap<QString, QVariant> constraintCondition(pugi::xml_node* itemNode);

	signals:
		// value changed
		void Signal_ValueChanged(const QString& propertyName, QVariant value);
		void Signal_ValueChanged(const QString& propertyName, QVariant value, int row, int column);

	private:
		QString						m_id;
		pugi::xml_document*			m_cfg;
		AttribList					m_valueList;
		vector<QStandardItem*>		m_standItems;
		map<QString, QStandardItem*>m_propertyStandItmes;
		bool						m_isEnableGB2312;
	};
}
