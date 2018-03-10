#include "LineWidget.h"
#include "SlideButton.h"
#include "BackgroundWidget.h"
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDrag>

namespace Studio
{
	static const int kFixedHeigth = 15;
	static const int kUnitWidth = 9;
	static const int kUnitHeight = 17;
	LineWidget::LineWidget(QString name, QWidget* parent /* = 0 */)
		: m_name(name), QWidget(parent)
	{
		setFixedHeight(kFixedHeigth);
		setAttribute(Qt::WA_TranslucentBackground);
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onCustomContextMenuRequested(QPoint)));
	}

	LineWidget::~LineWidget()
	{
	}

	SlideButton* LineWidget::getButton(int pos)
	{
		auto iter = m_items.begin(); 
		for (; iter != m_items.end(); ++iter)
		{
			SlideButton* button = *iter; 
			if (button->getPosition() == pos)
				return button; 
		}

		return nullptr; 
	}

	SlideButton* LineWidget::getButtonByIndex(int idx)
	{
		return m_items.at(idx); 
	}

	QRectF LineWidget::getRealRect(int yindex)
	{
		int	m_realXLeft = INT_MAX;
		int m_realXRight = INT_MIN;
		for (int i = 0; i < m_items.size(); i++)
		{
			if (m_items.at(i)->m_x < m_realXLeft)
			{
				m_realXLeft = m_items.at(i)->m_x;
			}
			if (m_items.at(i)->m_x > m_realXRight)
			{
				m_realXRight = m_items.at(i)->m_x;
			}
		}

		QRectF rect;
		rect.setX(m_realXLeft);
		rect.setY(yindex*kUnitHeight);
		int totalwidth = m_realXRight - m_realXLeft + 1;
		rect.setWidth(totalwidth + kUnitWidth);
		rect.setHeight(kUnitHeight); 
		return rect;
	}

	void LineWidget::onCustomContextMenuRequested(const QPoint& pos)
	{
		auto p = mapToParent(pos);
		int row = p.y() / 17;
		int column = p.x() / 9;
		QRectF rect = getRealRect(0);
		
		//判定右键点击是否在指定区域内
		if (p.x() >= rect.x() && p.x() <= rect.x() + rect.width())
		{
			emit customContextMenuRequested(row, column);
		}
	}

	bool LineWidget::onBackgroundWidgetClicked(QString text)
	{
		for (int i = 0; i < m_items.size(); i++)
		{
			auto* button = m_items[i]; 
			auto index = button->contain(text); 
			if (index > -1)
			{
				button->onManualClicked(index);
				return true; 
			}
		}

		return false; 
	}

	BackgroundWidget* LineWidget::getChainedWidget(SlideButton* button)
	{
		for (auto* background : m_anims)
		{
			if (button->contain(background->objectName()) != -1)
				return background;
		}

		return nullptr; 
	}

}  // namespace Studio
