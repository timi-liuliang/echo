#include <QtWidgets>
#include <QPushButton>
#include "Editpanel.h"
#include "LineWidget.h"
#include "BackgroundWidget.h"
#include "SlideButton.h"
#include "Studio.h"

namespace Studio
{
	static const int kBegin = 0;
	static const int kUnitWidth = 9;
	static const int kUnitHeight = 17;

	static const QColor kBackgroundColor(212,212,212);
	static const QColor kConnectLineBgColor(164, 212, 234);
	static const QColor kCurFrameLineColor(100, 0, 0);
	static const QColor kCurSelectedLayerBGColor(21,152,250);
	EditPanel::EditPanel(QWidget* parent)
		: QWidget(parent)
		, m_timelinePanel(nullptr)
		, m_CurrentFrame(0)
		, m_CurSelectedKeyRowIndex(-1)
		, m_CurSelectedKeyColumnIndex(-1)
		, m_CurSelectedLayerIndex(-1)
		, m_msPerFrame(0.02)
	{
		auto boxLayout = new QVBoxLayout(this);
		boxLayout->setContentsMargins(QMargins());
		boxLayout->setSpacing(2);
		boxLayout->addStretch();
		setLayout(boxLayout);
		setFixedWidth(kUnitWidth*2000);
	}

	int EditPanel::getPixelWidth(float second) const
	{
		int ms = second * 1000.f; 
		int frameCount = ms / m_msPerFrame; 

		return frameCount == 0 ? frameCount * kUnitWidth : frameCount * kUnitWidth - 3;
	}

	void EditPanel::addItem(const LineItem& item)
	{
		auto boxLayout = static_cast<QVBoxLayout*>(layout());
		
		LineWidget* background = new LineWidget(item.name(), this);
		
		QObject::connect(background, SIGNAL(customContextMenuRequested(int, int)), 
							getTimelinePanel(), SLOT(onCustomContextMenuRequested(int, int)));

		int buttonIndex = 0;

		// 先AddWidget
		for (int i = 0; i < item.size(); ++i)
		{
			const LineNode& node = item.node(i);
			if (node.duration != 0.f)
			{
				addDurationWidget(background, node); 
			}

		}

		// 再AddNode
		for (int i = 0; i < item.size(); ++i)
		{
			const LineNode& node = item.node(i);

			int buttonPosition = getPixelWidth(node.beginTime);
			addSlideButton(background, node, buttonPosition, buttonIndex);
		}

		boxLayout->insertWidget(m_items.size(), background);
		m_items.push_back(background);
	}

	void EditPanel::clear()
	{
		for (int i = 0; i < m_items.size(); ++i)
		{
			delete m_items.at(i);
		}
		m_items.clear();

		m_CurSelectedKeyRowIndex = -1; 
		m_CurSelectedKeyColumnIndex = -1; 
		m_CurrentFrame = 0;
		m_CurSelectedLayerIndex = -1;
	}

	LineWidget* EditPanel::getLine(int row)
	{
		return m_items.at(row); 
	}

	void EditPanel::SetTimePerFrame(unsigned int msPerFrame)
	{
		m_msPerFrame = msPerFrame; 
	}

	void EditPanel::UpdateVertTimeFrame(int time)
	{
		int x = width();
		int frame = time / m_msPerFrame;
		int drawFrame = frame - kBegin;
		if (drawFrame == m_CurrentFrame)
			return;
		m_CurrentFrame = drawFrame;
		update();
	}

	void EditPanel::UpdateUIKeyFramePos(int layerindex, int frameIndex, int offset)
	{
		LineWidget * widget = m_items.at(layerindex);
		SlideButton* button = widget->m_items.at(frameIndex);
		BackgroundWidget* background = widget->getChainedWidget(button); 

		button->UpdateUIKeyFramePos(offset);

		if (background)
			background->UpdateUIKeyFramePos(offset);
		
		update();
	}

	void EditPanel::setSelectedKeyFrame(int row, int index)
	{
		//点击的和之前选择的是同一帧,不处理
		if ((m_CurSelectedKeyRowIndex == row && m_CurSelectedKeyColumnIndex == index))
		{
			return;
		}

		if (m_CurSelectedKeyRowIndex != -1 && m_CurSelectedKeyColumnIndex != -1)
		{
			LineWidget* widget = m_items.at(m_CurSelectedKeyRowIndex);
			if (widget)
			{
				if (m_CurSelectedKeyColumnIndex > widget->m_items.size())
				{
					return;
				}
				SlideButton* key = widget->m_items.at(m_CurSelectedKeyColumnIndex);
				if (key)
				{
					//将之前选中关键帧的选中状态去掉
					key->setSelected(false);
				}
			}
		}
		//记录当前选中关键帧位置
		m_CurSelectedKeyRowIndex = row;
		m_CurSelectedKeyColumnIndex = index;
	}

	void EditPanel::setSelectedLayer(int row)
	{
		m_CurSelectedLayerIndex = row;
		update();
	}

	void EditPanel::paintEvent(QPaintEvent* event)
	{
		Q_UNUSED(event);

		int boundWidth = width();
		int boundHeight = height();

		QPainter painter(this);
		//painter.translate(QPointF(kUnitWidth, 0));
		painter.setPen(Qt::NoPen);
		painter.setBrush(kBackgroundColor);
		QVector<QRectF> rects;

		int kPointOne = 5 * kUnitWidth; 
		for (int i = 0; i < boundWidth; i += kPointOne)
		{
			QPointF topLeft(i, 0);
			QPointF bottomRight(i + kUnitWidth, boundHeight);
			rects.push_back(QRectF(topLeft, bottomRight));
		}
		painter.drawRects(rects);
		rects.clear();

		QVector<QLineF> lines;
		painter.setPen(kBackgroundColor);
		// horizontal
		for (int i = 0; i < boundHeight; i += kUnitHeight)
		{
			QPointF p1(0, i);
			QPointF p2(boundWidth, i);
			lines.push_back(QLineF(p1, p2));
		}
		// vertical
		for (int i = 0; i < boundWidth; i += kUnitWidth)
		{
			if ((i % kPointOne) != 0 && (i % kPointOne) != 1)
			{
				QPointF p1(i, 0);
				QPointF p2(i, boundHeight);
				lines.push_back(QLineF(p1, p2));
			}
		}
		painter.drawLines(lines);
		lines.clear();
		painter.setBrush(kConnectLineBgColor);

		//绘制蓝色背景
		if (!m_items.empty())
		{
			for (int i = 0; i < m_items.size(); i++)
			{
				QRectF rect = m_items[i]->getRealRect(i);
				rects.push_back(rect);

			}
		}
		painter.drawRects(rects);

		//绘制当前选中图层的背景
		if (!m_items.empty())
		{
			if (m_CurSelectedLayerIndex != -1 && m_CurSelectedLayerIndex < m_items.size())
			{
				QRectF rect = m_items[m_CurSelectedLayerIndex]->getRealRect(m_CurSelectedLayerIndex);
				painter.setBrush(kCurSelectedLayerBGColor);
				painter.drawRect(rect);
			}
		}	

		painter.setPen(Qt::black);
		//绘制关键帧连接线
		if (!m_items.empty())
		{
			for (int i = 0; i < m_items.size(); i++)
			{
				QRectF rect = m_items[i]->getRealRect(i);
				QPointF p1(rect.x() + kUnitWidth /2.0, (kUnitHeight / 2.0) + rect.y());
				QPointF p2(rect.x() + rect.width() - kUnitWidth/2.0,(kUnitHeight/2.0) + rect.y());
				lines.push_back(QLineF(p1, p2));
			}
		}
		painter.drawLines(lines);

		//绘制当前关键帧的标注线
		if (m_CurrentFrame != -1)
		{
			QPointF point1(m_CurrentFrame * kUnitWidth + kUnitWidth / 2.0 - 2, 0);
			QPointF point2(m_CurrentFrame * kUnitWidth + kUnitWidth / 2.0 - 2,  height());
			
			QPen pen(kCurFrameLineColor);
			pen.setWidth(2);

			painter.setPen(pen);
			painter.drawLine(point1, point2);
		}

	}

	QObject* EditPanel::getTimelinePanel()
	{
		if (!m_timelinePanel)
		{
			auto w = parent();
			while (w)
			{
				if (!w->objectName().compare(QStringLiteral("TimelinePanel")))
				{
					m_timelinePanel = w;
					break;
				}
				w = w->parent();
			}
		}
		return m_timelinePanel;
	}

	void EditPanel::addDurationWidget(LineWidget* background, const LineNode& node)
	{
		auto animation = new BackgroundWidget(background);
		animation->setIndex(m_items.size());
		QObject::connect(animation, SIGNAL(customContextMenuRequested(int, int)), getTimelinePanel(),	 SLOT(onCustomContextMenuRequested(int, int)));
		QObject::connect(animation, SIGNAL(widgetClicked(QString)),				  this,					 SLOT(onBackgroundWidgetClicked(QString)));
		animation->setText(node.text);
		animation->setObjectName(node.name);
		animation->setWidth(getPixelWidth(node.duration));
		animation->setPosition(getPixelWidth(node.beginTime));
		background->addSubWidget(animation);
		background->m_anims.push_back(animation); 
	}

	void EditPanel::addSlideButton(LineWidget* background, const LineNode& node, int position, int& buttonIdx)
	{
		auto* button = background->getButton(position);
		if (button)
		{
			button->append(SlideButton::SubData(node.name, node.color));
		}
		else
		{
			SlideButton* button = new SlideButton(background);
			button->setIndex(QPoint(m_items.size(), buttonIdx++));

			QObject::connect(button, SIGNAL(moved(int, int, float)),
				getTimelinePanel(), SLOT(onEventButtonMoved(int, int, float)), Qt::QueuedConnection);

			QObject::connect(button, SIGNAL(keyClicked(int, int, int)),
				getTimelinePanel(), SLOT(onEventButtonClicked(int, int, int)), Qt::QueuedConnection);

			if (node.color != Qt::red)
			{
				QObject::connect(button, SIGNAL(customContextMenuRequested(int, int)),
					getTimelinePanel(), SLOT(onCustomContextMenuRequested(int, int)));
			}

			button->setText(node.name);
			button->setPosition(getPixelWidth(node.beginTime));
			button->setObjectName("button");
			button->setColor(node.color);
			button->setIsLastKey(node.color == Qt::red);
			button->setParentPanel(this);

			background->addSubWidget(button);
			background->m_items.push_back(button);
		}

	}

	void EditPanel::onBackgroundWidgetClicked(QString text)
	{
		for (int i = 0; i < m_items.size(); i++)
		{
			if (m_items[i]->onBackgroundWidgetClicked(text))
				return; 
		}
	}

}  // namespace Studio
