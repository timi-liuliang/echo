#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	class QVector3Editor : public QWidget
	{
		Q_OBJECT

	public:
		QVector3Editor(class QPropertyModel* model, QString propertyName, QWidget* parent = 0);

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
		QLabel*			m_labelX;
		QLineEdit*		m_lineEditX;
		QLabel*			m_labelY;
		QLineEdit*		m_lineEditY;
		QLabel*			m_labelZ;
		QLineEdit*		m_lineEditZ;

		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}