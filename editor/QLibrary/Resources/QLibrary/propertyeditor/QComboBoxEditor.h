#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	class QComboBoxEditor : public QWidget
	{
		Q_OBJECT

	public:
		QComboBoxEditor(class QPropertyModel* model, QString propertyName, QWidget* parent = 0);

		// add item
		void addItem(const QString& val);

		// set value
		void setValue(const QString& val);

	public slots:
		// edit finished
		void onEditFinished();

	private:
		QHBoxLayout*	m_horizonLayout;
		QComboBox*		m_comboBox;;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}