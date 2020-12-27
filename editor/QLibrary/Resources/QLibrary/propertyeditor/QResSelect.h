#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QPainter>
#include <functional>
#include "engine/core/util/StringUtil.h"

namespace QT_UI
{
	//-------------------------------------------
	// AssetsSelect 2013-1-22 Captain
	//-------------------------------------------
	class QResSelect : public QWidget
	{
		Q_OBJECT

	public:
		QResSelect(class QPropertyModel* model, QString propertyName, const char* exts, const char* files, QWidget* parent = 0);

		// path
		void SetPath( const Echo::String text) { m_lineEdit->setText( text.c_str());  }
		QString GetPath() { return m_lineEdit->text(); }

		// MVC paint
		static bool ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val);

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

		// is texture res
		bool isTextureRes();

		// correct size
		void adjustHeightSize();

	private slots:
		// slots
		void onSelectPath();
		void onShowRes();
		void onEditFinished();

	private:
		Echo::String	m_exts;
		QString			m_files;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_resSelectButton = nullptr;
		QToolButton*	m_resFindButton = nullptr;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
	};
}