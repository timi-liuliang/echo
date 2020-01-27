#include "QColorSelect.h"
#include <QColorDialog>
#include <engine/core/util/StringUtil.h>

namespace QT_UI
{
	QColorSelect::QColorSelect( QWidget* parent)
		: QPushButton( parent)
	{ 
		connect( this, SIGNAL(clicked()), this, SLOT(OnSelectColor()));
	}

	void QColorSelect::OnSelectColor()
	{
		QColor color = QColorDialog::getColor(QColor((int)(m_color.r*255.f), (int)(m_color.g*255.f), (int)(m_color.b*255.f), (int)(m_color.a*255.f)), this, QString("Select Color"), QColorDialog::ShowAlphaChannel);
		if( color.isValid())
			SetColor( Echo::Color( color.red()/255.f, color.green()/255.f, color.blue()/255.f,color.alpha()/255.f));
	}

	void QColorSelect::SetColor( string color) 
	{ 
		m_color = Echo::StringUtil::ParseColor(color.c_str());
        
        emit Signal_ColorChanged();
	}

	void QColorSelect::SetColor( const Echo::Color& color)
	{ 
		m_color=color;
        
        emit Signal_ColorChanged();
	}

	string QColorSelect::GetColorStr()
	{
		string color = Echo::StringUtil::ToString(m_color);
		return color;
	}

    static bool ItemDelegatePaintColor( QPainter *painter, const QRect& rect, const string& val)
    {
        Echo::Color color = Echo::StringUtil::ParseColor(val.c_str());
        color = color * color.a + Echo::Color::WHITE * (1.f - color.a);
        
        QColor qColor((int)(color.r*255.f), (int)(color.g*255.f), (int)(color.b*255.f));

        // color rect
        QRect tRect = QRect( rect.left()+3, rect.top()+3, rect.height()-6, rect.height()-6);
        painter->setBrush( qColor);
        painter->drawRect( tRect);
        painter->setPen( QColor( 0, 0, 0));
        painter->drawRect( tRect);

        return true;
    }

    static bool ItemDelegatePaintText( QPainter *painter, const QRect& rect, const string& val)
    {
        Echo::Color color = Echo::StringUtil::ParseColor(val.c_str());
        QColor qColor((int)(color.r*255.f), (int)(color.g*255.f), (int)(color.b*255.f));

        // text
        Echo::String text = Echo::StringUtil::Format( "rgba(%d,%d,%d,%d)", (int)(color.r*255.f), (int)(color.g*255.f), (int)(color.b*255.f),(int)(color.a*255.f));
        QRect textRect( rect.left()+rect.height()+3, rect.top()+3, rect.width()-rect.height()-3, rect.height()-6);
        QFont font = painter->font(); font.setBold(false);
        painter->setFont(font);
        painter->setPen(QColor( 232, 232, 232));
        painter->drawText( textRect, Qt::AlignLeft, text.c_str());

        return true;
    }


	bool QColorSelect::ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val)
	{
        ItemDelegatePaintText(painter, rect, val);
        ItemDelegatePaintColor(painter, rect, val);

		return true;
	}

	QSize QColorSelect::sizeHint() const
	{
		return size();
	}

	void QColorSelect::paintEvent( QPaintEvent* event)
	{
		QPainter painter( this);

		ItemDelegatePaintColor( &painter, rect(), GetColorStr());
        
        if(m_isDrawText)
            ItemDelegatePaintText(&painter, rect(), GetColorStr());
	}
}
