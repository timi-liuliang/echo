#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

namespace QT_UI
{
	/**
	 * FileSelect captain 2013-1-22
	 */
	class QFileSelect : public QWidget
	{
		Q_OBJECT
	public:
		QFileSelect( QWidget* parent = 0);

		// set path
		void SetPath( QString text) { m_lineEdit->setText( text);  }

		// get path
		QString GetPath() { return m_lineEdit->text(); }

	private slots:
		// onn select
		void OnSelectPath();

	private:
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
	};
}
