#pragma once

#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
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
		QResEditor(class QPropertyModel* model, QString propertyName, const char* resTypes, QWidget* parent);

		// Id
		void SetId( QString text) { m_id = Echo::StringUtil::ParseI32(text.toStdString().c_str());  }
		QString GetId() { return Echo::StringUtil::ToString(m_id).c_str(); }

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
		Echo::i32			m_id;
		Echo::StringArray	m_resTypes;
		Echo::String		m_exts;
		QHBoxLayout*		m_horizonLayout;
		QPushButton*		m_displayButton;
		QToolButton*		m_toolButton;
		QPropertyModel*		m_propertyModel;
		QString				m_propertyName;
		QMenu*				m_menu;			// Mouse right button click
	};
}
