#include "SlideButton.h"
#include "Editpanel.h"
#include <QMouseEvent>
#include <QDebug>
#include <QtWidgets>

namespace Studio
{
	static const int kFixedHeigth = 16;
	static const int kUnitWidth = 9;
	SlideButton::SlideButton(QWidget* parent /* = 0 */)
		: QPushButton(parent)
		, m_x(0)
		, m_width(9)
		, m_index(QPoint(-1, -1))
		, m_bIsLastKey(false)
		, m_parentPanel(NULL)
		, m_selected(false)
		, m_color(Qt::white)
	{
		setFixedSize(m_width, kFixedHeigth);
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
	}

	SlideButton::~SlideButton()
	{
	}

	void SlideButton::setPosition(int x)
	{
		m_x = x;
	}

	int SlideButton::getPosition() const
	{
		return m_x;
	}

	int SlideButton::getFrame() const
	{
		return m_x / kUnitWidth;
	}

	void SlideButton::setWidth(int width)
	{
		m_width = width;
		setFixedSize(m_width, kFixedHeigth);
	}

	void SlideButton::setIsLastKey(bool flag)
	{
		m_bIsLastKey = flag;
	}

	void SlideButton::setIndex(const QPoint& index)
	{
		m_index = index;
	}

	void SlideButton::setColor(QColor color)
	{
		m_color = color; 
	}

	void SlideButton::setParentPanel(EditPanel* panel)
	{
		m_parentPanel = panel; 
	}

	void SlideButton::setSelected(bool flag)
	{
		m_selected = flag; 
	}

	void SlideButton::append(const SubData& sub)
	{
		m_subDatas.push_back(sub); 
	}

	const QList<SlideButton::SubData>& SlideButton::getSubDatas() const
	{
		return m_subDatas; 
	}

	void SlideButton::mousePressEvent(QMouseEvent *e)
	{
		m_mouseMovePos = QPoint();
		m_mousePressPos = QPoint();
		if (e->button() ==  Qt::LeftButton)
		{
			m_mouseMovePos = e->globalPos();
			m_mousePressPos = e->globalPos();
			//设置自身被选中，并将消息传给关键帧区域绘制面板，让其更新相应设置
			m_selected = true;
		}

		if (m_parentPanel)
		{
			m_parentPanel->setSelectedKeyFrame(m_index.x(), m_index.y());
		}

		QPushButton::mousePressEvent(e);
	}

	void SlideButton::mouseMoveEvent(QMouseEvent *e)
	{
		if (e->buttons() == Qt::LeftButton)
		{
			auto curPos = parentWidget()->mapFromGlobal(e->globalPos());
			curPos.setY(0);
			if (curPos.x() < 0)
			{
				curPos.setX(0);
			}
			curPos.setX(curPos.x() / m_width);
			emit moved(m_index.x(), m_index.y(), curPos.x());
		}
		QPushButton::mouseMoveEvent(e);
	}

	void SlideButton::mouseReleaseEvent(QMouseEvent *e)
	{
		if (e->button() == Qt::LeftButton && !m_bIsLastKey)
		{
			emit keyClicked(m_index.x(), m_index.y(), 0);
		}
		if (m_mousePressPos != QPoint())
		{
			auto moved = e->globalPos() - m_mousePressPos;
			if (moved.manhattanLength() > m_width)
			{
				e->ignore();
				return;
			}
		}
		QPushButton::mouseReleaseEvent(e);
	}

	void SlideButton::UpdateUIKeyFramePos(int offset)
	{
		QPoint pos(pos().x() + offset*kUnitWidth,0);
		move(pos);
		setPosition(pos.x());
	}

	void SlideButton::paintEvent(QPaintEvent* event)
	{
		Q_UNUSED(event);

		int w = width()-3;
		int h = height()-4;

		QPainter painter(this);
		painter.translate(QPointF(2,2));
		QRectF rect(0,0,w,h);

		//最后一帧红底黑框
		if (m_bIsLastKey)
		{
			painter.setBrush(Qt::red);
			painter.drawRect(rect);
			painter.setPen(Qt::black);
			painter.drawRect(rect);
		}

		//选中状态需要重画边框
		else if (m_selected)
		{
			painter.setBrush(m_color);
			painter.drawRect(rect);
			QPen pen(QColor(15, 185, 96));
			pen.setWidth(1);
			painter.setPen(pen);
			painter.drawRect(rect);
		}
		else
		{
			//正常状态 自定义颜色 + 黑框 + 多重事件
			int eventCount = m_subDatas.size() + 1; // main + sub; 
			
			int perh = h / eventCount;

			QRectF subRect(0, 0, w, perh);

			for (int i = 0; i < m_subDatas.size(); i++)
			{
				const auto& data = m_subDatas.at(i); 
				
				painter.translate(QPointF(0, i == 0 ? 0 : perh)); 

				painter.setBrush(data.color); 
				painter.drawRect(subRect);
				painter.setPen(Qt::black);
				painter.drawRect(subRect);
			}

			painter.translate(QPointF(0, m_subDatas.empty() ? 0 : perh));
			painter.setBrush(m_color); 
			painter.drawRect(subRect);
			painter.setPen(Qt::black); 
			painter.drawRect(subRect); 
		}
	}

	void SlideButton::onCustomContextMenuRequested(const QPoint& p)
	{
		auto pos = mapTo(parentWidget()->parentWidget(), p);
		int row = pos.y() / 17;
		int column = pos.x() / 9;
		emit customContextMenuRequested(row, column);
	}

	void SlideButton::onManualClicked(int subIndex)
	{
		emit keyClicked(m_index.x(), m_index.y(), subIndex);
	}

	int SlideButton::contain(QString text)
	{
		if (this->text() == text)
		{
			return 0; 
		}

		for (int i = 0; i < m_subDatas.size(); i++)
		{
			if (m_subDatas[i].name == text)
			{
				return i + 1; 
			}
		}

		return -1; 
	}

	QString SlideButton::getName(int subindex) const
	{
		return subindex == 0 ? this->text() : m_subDatas[subindex - 1].name;
	}

}  // namespace Studio
