#pragma once

#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <engine/core/math/color.h>

using namespace std;

namespace QT_UI
{
	class QChannelEditor : public QPushButton
	{
		Q_OBJECT
	public:
		QChannelEditor( QWidget* parent = 0);

		// set|get expression
		void SetExpression( string color);
		string GetExpression();

		// paint
		static bool ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);

	protected:
		// paint event
		void paintEvent( QPaintEvent* event); 

		// size
		QSize sizeHint() const;

	public slots:
		// edit event
		void onSelectColor();

	private:
		Echo::String		m_expression;
	};
}
