#include <QtWidgets>
#include "Ruler.h"
#include <engine/core/Base/TypeDef.h>

namespace Studio
{
	static const int kBegin = 0;
	static const int kUnitWidth = 9;
	static const int kUnitHeight = 33;
	static const int kFrameCountPerText = 10; 
	static const int kMSecondPerSecond = 1000; 
	static const int kOneText = kFrameCountPerText * kUnitWidth;
	static const int kMainTickLength = 8;
	static const int kSubTickLength = 5;
	static const int kRulerHeight = 35;

	static const QColor kMainTickColor(Qt::black);
	static const QColor kSubTickColor(Qt::black);
	static const QColor kCurFrameBgColor(255, 125, 125, 125);
	static const QColor kCurFrameLineColor(100, 0, 0);

	Ruler::Ruler(QWidget* parent)
		: QWidget(parent), m_CurrentFrame(0), offsetValue(0), m_isUsingFloat(false)
	{
		setFixedHeight(kRulerHeight);

		m_msPerFrame = 20; 
		m_framePerSecond = kMSecondPerSecond / m_msPerFrame; 
	}

	void Ruler::SetTimePerFrame(unsigned int msPerFrame)
	{
		m_msPerFrame = msPerFrame;
	}

	uint Ruler::getTimePerFrame()
	{
		return m_msPerFrame;
	}

	//更新面板的时间纵轴.
	void Ruler::UpdateVertTimeFrame(int time)
	{
		int frame = time / m_msPerFrame;
		int drawFrame = frame - kBegin;
		if (drawFrame == m_CurrentFrame)
			return;

		m_CurrentFrame = drawFrame;
		update();
	}

	void Ruler::mousePressEvent(QMouseEvent *e)
	{
		if (e->button() == Qt::LeftButton)
		{
			QPoint localPos = mapFromGlobal(e->globalPos());

			int selectFrame = (localPos.x() + offsetValue) / kUnitWidth;
			Echo::i32 startTime = ((selectFrame + 1 / 2) * m_msPerFrame); 
			emit frameClicked(startTime);
		}
	}

	void Ruler::on_editPanelScrollArea_horizontalScroll(int value)
	{
		offsetValue = value;
		update();
		//move(-value, y());
	}

	void Ruler::paintEvent(QPaintEvent* event)
	{
		Q_UNUSED(event);

		int side =  width();

		QPainter painter(this);
		//painter.translate(QPointF(kUnitWidth, 0));

		QVector<QLineF> lines;
		painter.setPen(kMainTickColor);
		for (int i = 0; i < side; i += kOneText)
		{
			QPointF p1(i, 0);
			QPointF p2(i, kMainTickLength);
			QPointF p3(i, height());
			QPointF p4(i, height() - kMainTickLength);
			lines.push_back(QLineF(p1, p2));
			lines.push_back(QLineF(p3, p4));
			QRectF rect(p2, QPointF(i + kOneText, height() - kMainTickLength));

			int textCount = (i + offsetValue) / kOneText;
			
			int frame = textCount * kFrameCountPerText;
			Echo::Real second = frame * m_msPerFrame / 1000.f; 

			QString frameText = m_isUsingFloat ? QString::number(second, 'g', 3) : QString::number(frame);
			painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, frameText);
		}
		painter.drawLines(lines);

		lines.clear();
		painter.setPen(kSubTickColor);

		for (int i = 0; i < side; i += kUnitWidth)
		{
			if ((i % kOneText) != 0)
			{
				QPointF p1(i, 0);
				QPointF p2(i, kSubTickLength);
				QPointF p3(i, height());
				QPointF p4(i, height() - kSubTickLength);
				lines.push_back(QLineF(p1, p2));
				lines.push_back(QLineF(p3, p4));
			}
		}
		painter.drawLines(lines);

		painter.setBrush(kCurFrameBgColor);
		//绘制当前帧标注外框及标注线
		if (m_CurrentFrame != -1)
		{
			//框
			QRectF rect;
			rect.setX(m_CurrentFrame * kUnitWidth - offsetValue);
			rect.setY(0);
			rect.setWidth(kUnitWidth);
			rect.setHeight(kUnitHeight);
			painter.drawRect(rect);
			
			//线
			QPointF point1(m_CurrentFrame * kUnitWidth + kUnitWidth/2.0 - offsetValue, 0);
			QPointF point2(m_CurrentFrame * kUnitWidth + kUnitWidth/2.0 - offsetValue,kUnitHeight + 2);
			painter.setPen(kCurFrameLineColor);
			QPen pen(kCurFrameLineColor); 
			pen.setWidth(2); 
			painter.setPen(pen); 
			painter.drawLine(point1, point2);
		}

	}
}  // namespace Studio

