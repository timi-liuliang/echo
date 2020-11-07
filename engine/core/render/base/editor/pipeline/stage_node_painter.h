#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "engine/core/main/Engine.h"
#include "custom/qgraphics_pixmap_item_custom.h"

namespace Pipeline
{
	struct StatgeNodePainter
	{
		struct 
		{
			QColor m_normalBoundaryColor = QColor(137, 137, 137);
			QColor m_selectedBoundaryColor = QColor(255, 165, 0);
			QColor m_gradientColor0 = Qt::gray;
			QColor m_gradientColor1 = QColor(80, 80, 80);
			QColor m_gradientColor2 = QColor(64, 64, 64);
			QColor m_gradientColor3 = QColor(58, 58, 58);
			QColor m_shadowColor	= QColor(20, 20, 20);
			QColor m_fontColor		= Qt::gray;
			QColor m_fontColorFaded = Qt::gray;
			QColor m_warningColor = QColor(128, 128, 0);
			QColor m_errorColor	  = Qt::red;
			QColor finalColor	  = QColor(54, 108, 179, 255);
			float  m_penWidth = 1.5f;
			float  m_cornerRadius = 0.f;
		}									m_style;
		Echo::RenderStage*					m_stage = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsPathItem*					m_rect = nullptr;
		QGraphicsPixmapItem*				m_nextArrow = nullptr;
		QGraphicsPixmapItemCustom*			m_addAction = nullptr;
		size_t								m_renderQueueSize = 0;
		float								m_rectFinalWidth = 15;
		float								m_width = 190;
		float								m_height = 240;
		QGraphicsSimpleTextItem*			m_text = nullptr;

		StatgeNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderStage* stage)
		{
			m_stage = stage;
			m_graphicsView = view;
			m_graphicsScene = scene;

			if (!m_rect)
			{
				float halfWidth = m_width * 0.5f;

				m_rect = new QGraphicsPathItem(nullptr);
				m_rect->setZValue(-1.f);
				m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
				m_rect->setFlag(QGraphicsItem::ItemIsMovable, true);
				m_rect->setPos(QPointF(0.f, 0.f));
				m_graphicsScene->addItem(m_rect);

				Echo::Vector2 textPos(15.f, 15.f);
				m_text = m_graphicsScene->addSimpleText(m_stage->getName().c_str());
				m_text->setBrush(QBrush(m_style.m_fontColor));
				m_text->setParentItem(m_rect);
				m_text->setPos(textPos.x - halfWidth, textPos.y);

				QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/right-arrow.png").c_str());
				m_nextArrow = m_graphicsScene->addPixmap(rightArrow.scaled(QSize(16, 16)));
				m_nextArrow->setParentItem(m_rect);
				m_nextArrow->setPos(QPointF(halfWidth + 5.f, 0.f));

				QPixmap addNew((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/import_dark.png").c_str());
				m_addAction = new QGraphicsPixmapItemCustom();
				m_addAction->setPixmap(addNew.scaled(QSize(16, 16)));
				m_addAction->setParentItem(m_rect);
				m_addAction->setAcceptHoverEvents(true);
				m_graphicsScene->addItem(m_addAction);
			
				m_addAction->setHoverEnterEventCb([](QGraphicsPixmapItem* item)
				{
					QPixmap addNew((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/import.png").c_str());
					item->setPixmap(addNew.scaled(QSize(16, 16)));
				});

				m_addAction->setHoverEnterLeaveCb([](QGraphicsPixmapItem* item)
				{
					QPixmap addNew((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/import_dark.png").c_str());
					item->setPixmap(addNew.scaled(QSize(16, 16)));
				});
			}
		}

		~StatgeNodePainter()
		{
			reset();
		}

		// reset
		void reset()
		{
			if (m_rect)
				m_graphicsScene->removeItem(m_rect);

			m_stage = nullptr;
			m_graphicsView = nullptr;
			m_graphicsScene = nullptr;
			m_rect = nullptr;
			m_text = nullptr;
		}

		// set
		void set()
		{

		}

		// update
		void update(Echo::i32 xPos, bool isFinal)
		{
			if (m_rect)
			{		
				if (m_renderQueueSize != m_stage->getRenderQueues().size())
				{
					m_renderQueueSize = m_stage->getRenderQueues().size();
					m_height = 40.f + m_renderQueueSize * 56.f + 16.f;

					float halfWidth = m_width * 0.5f;
					float halfHeight = m_height * 0.5f;

					QPainterPath path;
					path.addRoundedRect(QRectF(-halfWidth, 0.f, m_width, m_height), m_style.m_cornerRadius, m_style.m_cornerRadius);
					m_rect->setPath(path);

					QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
					gradient.setColorAt(0.0, m_style.m_gradientColor0);
					gradient.setColorAt(0.03, m_style.m_gradientColor1);
					gradient.setColorAt(0.97, m_style.m_gradientColor2);
					gradient.setColorAt(1.0, m_style.m_gradientColor3);
					m_rect->setBrush(gradient);

					m_addAction->setPos(QPointF(-8.f, m_height - 24.f));
				}

				m_rect->setPos(xPos * 240.f, 0.f);
				m_nextArrow->setVisible(!isFinal);
			}
		}
	};
	typedef Echo::vector<StatgeNodePainter*>::type StageNodePainters;
}

#endif