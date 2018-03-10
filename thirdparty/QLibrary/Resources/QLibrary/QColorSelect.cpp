#include "QColorSelect.h"
#include <QColorDialog>

namespace QT_UI
{
	QColorSelect::QColorSelect( QWidget* parent)
		: QPushButton( parent)
	{ 
		// 消息
		connect( this, SIGNAL(clicked()), this, SLOT(OnSelectColor()));
	}

	// 选择颜色
	void QColorSelect::OnSelectColor()
	{
		QColor color = QColorDialog::getColor(QColor((int)(m_color.r*255.f), (int)(m_color.g*255.f), (int)(m_color.b*255.f), (int)(m_color.a*255.f)), this, QString("选择颜色"), QColorDialog::ShowAlphaChannel);
		if( color.isValid())
			SetColor( Echo::Color( color.red()/255.f, color.green()/255.f, color.blue()/255.f,color.alpha()/255.f));
	}

	// 设置路径
	void QColorSelect::SetColor( string color) 
	{ 
		m_color = Echo::StringUtil::ParseColor(color.c_str());
	}

	// 设置颜色
	void QColorSelect::SetColor( const Echo::Color& color)
	{ 
		m_color=color;
	}

	// 获取路径
	string QColorSelect::GetColor() 
	{
		string color = Echo::StringUtil::ToString(m_color);
		return color;
	}

	// MVC渲染
	void QColorSelect::ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val)
	{
		Echo::Color color = Echo::StringUtil::ParseColor(val.c_str());
		QColor qColor((int)(color.r*255.f), (int)(color.g*255.f), (int)(color.b*255.f));

		// 绘制颜色块
		QRect tRect =  QRect( rect.left()+3, rect.top()+2, rect.height()-4, rect.height()-4);// rect;  tRect.setWidth( rect.height()-4); tRect.setHeight( rect.height()-4);
		painter->setBrush( qColor);
		painter->drawRect( tRect);
		painter->setPen( qColor);
		painter->drawRect( tRect);

		// 文字显示
		Echo::String text = Echo::StringUtil::Format( "rgb(%d,%d,%d)"/* "#%02x%02x%02x%02x"*/, (int)(color.r*255.f), (int)(color.g*255.f), (int)(color.b*255.f),(int)(color.a*255.f));
		QRect textRect( rect.left()+rect.height()+3, rect.top()+2, rect.width()-rect.height()-3, rect.height()-4);
		painter->drawText( textRect, Qt::AlignLeft, text.c_str());
	}

	// 大小
	QSize QColorSelect::sizeHint() const
	{
		QSize tsize = size();

		return size();
	}

	// 重定义渲染
	void QColorSelect::paintEvent( QPaintEvent* event)
	{
		QPainter painter( this);

		ItemDelegatePaint( &painter, rect(), GetColor());
	}
}