#include "QChannelEditor.h"
#include "QCheckBoxEditor.h"
#include <QColorDialog>
#include <engine/core/util/StringUtil.h>
#include "ChannelExpressionDialog.h"

namespace QT_UI
{
	QChannelEditor::QChannelEditor( QWidget* parent)
		: QPushButton( parent)
	{ 
		connect( this, SIGNAL(clicked()), this, SLOT(OnSelectColor()));
	}

	void QChannelEditor::onSelectColor()
	{
		Echo::StringArray	dataArray = Echo::StringUtil::Split(m_info.c_str(), "#");
		Echo::String		expression = dataArray[0];
		if (Studio::ChannelExpressionDialog::getExpression(this, expression))
		{
			dataArray[0] = expression;

			setInfo(Echo::StringUtil::Format("%s#%s#%s", dataArray[0].c_str(), dataArray[1].c_str() ,dataArray[2].c_str()).c_str());
		}
	}

	void QChannelEditor::setInfo( const string& info)
	{ 
		m_info = info.c_str();
	}

	const string& QChannelEditor::getInfo()
	{
		return m_info;
	}

	bool QChannelEditor::ItemDelegatePaint( QPainter *painter, const QRect& rect, const string& val)
	{
		Echo::StringArray	dataArray = Echo::StringUtil::Split(val.c_str(), "#");
		Echo::String		expression = dataArray[0];
		Echo::String		value = dataArray[1];
		Echo::Variant::Type type = Echo::Variant::Type(Echo::StringUtil::ParseI32(dataArray[2]));

		// color rect
		QRect tRect =  QRect( rect.left()+1, rect.top()+1, rect.width()-2, rect.height()-2);
		painter->setBrush(QColor(70, 140, 70));
		painter->drawRect( tRect);
		painter->setPen( QColor( 0, 0, 0));
		painter->drawRect(QRect(rect.left(), rect.top(), rect.width() - 1, rect.height()-1));

		if (type == Echo::Variant::Type::Bool)
		{
			QCheckBoxEditor::ItemDelegatePaint(painter, rect, value);
		}
		else
		{
			// text
			Echo::String text = value.c_str();
			QRect textRect(rect.left() + 6, rect.top() + 3, rect.width() - 6, rect.height() - 6);
			QFont font = painter->font(); font.setBold(false);
			painter->setFont(font);
			painter->setPen(QColor(232, 232, 232));
			painter->drawText(textRect, Qt::AlignLeft, text.c_str());
		}

		return true;
	}

	QSize QChannelEditor::sizeHint() const
	{
		return size();
	}

	void QChannelEditor::paintEvent( QPaintEvent* event)
	{
		QPainter painter( this);

		ItemDelegatePaint( &painter, rect(), m_info.c_str());
	}
}
