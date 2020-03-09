#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QWidget>
#include <QPainter>
#include <QMenu>
#include <functional>
#include "engine/core/util/StringUtil.h"

namespace QT_UI
{
	class QResEditor : public QWidget
	{
		Q_OBJECT

	public:
		QResEditor(class QPropertyModel* model, QString propertyName, const char* resType, QWidget* parent);

		// Id
		void SetId( QString text) { m_lineEdit->setText( text);  }
		QString GetId() { return m_lineEdit->text(); }

		// MVC Paint
		static bool ItemDelegatePaint(QPainter *painter, const QRect& rect, const Echo::String& val);

	protected:
		// redefine paintEvent
		void paintEvent(QPaintEvent* event);

		// is texture res
		bool isTextureRes();

		// correct size
		void adjustHeightSize();

	private slots:
		// show menu
		void onShowMenu();

		// edit finished
		void onEditFinished();

		// node tree widget show menu
		void showMenu(const QPoint& point);

		// res operation
        void onCreateRes();
		void onLoad();
		void onEdit();
		void onClearRes();
        void onSaveRes();

	private:
		Echo::String	m_id;
		Echo::String	m_resType;
		Echo::String	m_exts;
		QHBoxLayout*	m_horizonLayout;
		QLineEdit*		m_lineEdit;
		QToolButton*	m_toolButton;
		QPropertyModel* m_propertyModel;
		QString			m_propertyName;
		QMenu*			m_menu;			// Mouse right button click
	};
}
