#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	class QRealEditor : public QWidget
	{
		Q_OBJECT

	public:
		QRealEditor(class QPropertyModel* model, QString propertyName, QWidget* parent = 0);

		// set value
		void setValue(const QString& val);

	public slots:
		// edit finished
		void onEditFinished();

		// editing
		void onEditing();

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}