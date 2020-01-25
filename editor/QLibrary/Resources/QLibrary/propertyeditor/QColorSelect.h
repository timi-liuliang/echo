#pragma once

#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <engine/core/math/color.h>

using namespace std;

namespace QT_UI
{
	//-------------------------------------------
	// ColorSelect 2013-3-4
	//-------------------------------------------
	class QColorSelect : public QPushButton
	{
		Q_OBJECT
	public:
		QColorSelect( QWidget* parent = 0);
        
        // draw text
        void setDrawText(bool isDrawText) { m_isDrawText = isDrawText; }

		// color by string
		void SetColor( string color);
        string GetColor();
        
        // color
		void SetColor( const Echo::Color& color);
		const Echo::Color& GetColor() const { return m_color; }

		// MVC
		static bool ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val);

	protected:
		// paint event
		void paintEvent( QPaintEvent* event);

		// size hint
		QSize sizeHint() const;

	public slots:
		// on select color
		void OnSelectColor();
        
    signals:
        //  signal color changed
        void Signal_ColorChanged();

	private:
        bool            m_isDrawText = true;
		Echo::Color		m_color;
	};
}
