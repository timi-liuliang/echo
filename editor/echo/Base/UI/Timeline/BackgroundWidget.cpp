#include "BackgroundWidget.h"
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

namespace Studio
{
	static const int kLineHeight = 17;
	static const int kUnitWidth = 9; 

	BackgroundWidget::BackgroundWidget(QWidget* parent /*= 0*/)
		: QWidget(parent)
		, m_x(0)
		, m_width(0)
		, m_index(-1)
	{
		setContentsMargins(QMargins());
		setFixedSize(m_width, kLineHeight);
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
	}

	BackgroundWidget::~BackgroundWidget()
	{
	}

	void BackgroundWidget::setPosition(int x)
	{
		m_x = x;
	}

	void BackgroundWidget::setWidth(int width)
	{
		m_width = width;
		setFixedSize(m_width, kLineHeight);
	}

	void BackgroundWidget::setText(const QString& text)
	{
		m_text = text;
	}

	void BackgroundWidget::setIndex(int index)
	{
		m_index = index;
	}

	void BackgroundWidget::mouseReleaseEvent(QMouseEvent* e)
	{
		if (e->button() == Qt::LeftButton)
		{
			emit widgetClicked(this->objectName());
		}

		QWidget::mouseReleaseEvent(e); 
	}

	void BackgroundWidget::paintEvent(QPaintEvent *e)
	{
		int w = width() - 3;
		int h = height() - 4 - 4;

		QPainter painter(this);
		painter.translate(QPointF(2, 4));
		QRectF rect(0, 0, w, h);

		painter.setBrush(Qt::cyan);
		painter.drawRect(rect);
		painter.setPen(Qt::black);
		painter.drawRect(rect);

		painter.setFont(QFont(QString::fromLocal8Bit("Consolas")));
		painter.drawText(rect, Qt::AlignCenter, m_text);
		QWidget::paintEvent(e);
	}

	void BackgroundWidget::onCustomContextMenuRequested(const QPoint& p)
	{
		auto point = mapToParent(p);
		qDebug() << "BackgroundWidget::onCustomContextMenuRequested: " << point;
		int row = m_index;
		int column = point.x() / 9;
		emit customContextMenuRequested(row, column);
	}

	void BackgroundWidget::UpdateUIKeyFramePos(int offset)
	{
		QPoint pos(pos().x() + offset*kUnitWidth, 0);
		move(pos);
		setPosition(pos.x());
	}

}  // namespace Studio

