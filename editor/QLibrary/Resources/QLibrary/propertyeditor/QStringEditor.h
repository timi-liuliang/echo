#pragma once

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	class QStringEditor : public QWidget
	{
		Q_OBJECT

	public:
		QStringEditor(class QPropertyModel* model, QString propertyName, const Echo::String& extraData, QWidget* parent = 0);

		// set value
		void setValue(const QString& val);

	public slots:
		// edit finished
		void onEditFinished();

		// editing
		void onEditing();

		// edit text
		void onEditText();

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

		// parse extra data
		void parseExtraData();

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
		Echo::String	m_extraData;
		bool			m_isBase64String = false;
		bool			m_readOnly = false;
		Echo::String	m_language;
	};
}
