#pragma once

#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <engine/core/math/color.h>
#include <engine/core/base/variant.h>

using namespace std;

namespace QT_UI
{
	class QChannelEditor : public QPushButton
	{
		Q_OBJECT

	public:
		QChannelEditor(class QPropertyModel* model, QString propertyName, QWidget* parent = 0);

		// set|get expression
		void setInfo( const string& info);
		const string& getInfo();

		// paint
		static bool ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);
		static bool ItemDelegatePaintExpression(QPainter *painter, const QRect& rect, const string& val, bool isRenderExpressionOnly);

	protected:
		// paint event
		void paintEvent( QPaintEvent* event); 

		// size
		QSize sizeHint() const;

	protected:
		// double click event
		virtual void mouseDoubleClickEvent(QMouseEvent* event);

	public slots:
		// edit event
		void onDisplayExpression();

	private:
		QPropertyModel*		m_propertyModel = nullptr;
		QString				m_propertyName;
		Echo::String		m_info;
		bool				m_displayExpression = true;
	};
}
