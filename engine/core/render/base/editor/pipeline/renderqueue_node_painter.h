#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"

namespace Pipeline
{
	struct RenderQueueNodePainter
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
			QColor m_connectionPointColor = QColor(169, 169, 169);
			QColor m_filledConnectionPointColor = Qt::cyan;
			QColor m_warningColor = QColor(128, 128, 0);
			QColor m_errorColor	  = Qt::red;
			QColor finalColor	  = QColor(54, 108, 179, 255);
			float  m_penWidth = 1.5f;
			float  m_cornerRadius = 0.f;
		}									m_style;
		Echo::IRenderQueue*					m_renderQueue = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsPathItem*					m_rect = nullptr;
		float								m_rectFinalWidth = 15;
		Echo::vector<QGraphicsItem*>::type	m_inputConnectionPoints;
		Echo::vector<QGraphicsItem*>::type	m_outputConnectionPoints;
		float								m_width = 160;
		float								m_height = 40;
		float								m_connectPointRadius = 8.f;
		Echo::Color							m_connectPointColor = Echo::Color::DARKCYAN;
		QGraphicsSimpleTextItem*			m_text = nullptr;

		RenderQueueNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::IRenderQueue* queue)
		{
			m_renderQueue = queue;
			m_graphicsView = view;
			m_graphicsScene = scene;

			if (!m_rect)
			{
				float halfWidth = m_width * 0.5f;
				float halfHeight = m_height * 0.5f;

				QPainterPath path;
				path.addRoundedRect(QRectF(-halfWidth, -halfHeight, m_width, m_height), m_style.m_cornerRadius, m_style.m_cornerRadius);

				m_rect = new QGraphicsPathItem(nullptr);
				m_rect->setPath(path);
				m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
				QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
				gradient.setColorAt(0.0,  m_style.m_gradientColor0);
				gradient.setColorAt(0.03, m_style.m_gradientColor1);
				gradient.setColorAt(0.97, m_style.m_gradientColor2);
				gradient.setColorAt(1.0,  m_style.m_gradientColor3);
				m_rect->setBrush(gradient);
				m_rect->setFlag(QGraphicsItem::ItemIsMovable, true);
				m_rect->setPos(QPointF(0.f, 0.f));
				m_graphicsScene->addItem(m_rect);

				m_text = m_graphicsScene->addSimpleText(m_renderQueue->getName().c_str());
				m_text->setBrush(QBrush(m_style.m_fontColor));
				m_text->setParentItem(m_rect);

				Echo::Rect textRect;
				EditorApi.qGraphicsItemSceneRect(m_text, textRect);
				m_text->setPos((m_width - textRect.getWidth()) * 0.5f - halfWidth, (m_height - textRect.getHeight()) * 0.5f - halfHeight);

				float halfConnectPointRadius = m_connectPointRadius * 0.5f;
				m_inputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, 0.f, 0.f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
				for (QGraphicsItem* item : m_inputConnectionPoints)
				{
					item->setParentItem(m_rect);
					item->setPos(0.f - halfConnectPointRadius, -halfHeight - halfConnectPointRadius * 3.f);
				}

				m_outputConnectionPoints.push_back(EditorApi.qGraphicsSceneAddEclipse(m_graphicsScene, 0.f, 0.f, m_connectPointRadius, m_connectPointRadius, m_connectPointColor));
				for (QGraphicsItem* item : m_outputConnectionPoints)
				{
					item->setParentItem(m_rect);
					item->setPos(0.f - halfConnectPointRadius, halfHeight + halfConnectPointRadius);
				}
			}
		}

		~RenderQueueNodePainter()
		{
			reset();
		}

		// reset
		void reset()
		{
			if (m_rect)
				m_graphicsScene->removeItem(m_rect);

			m_inputConnectionPoints.clear();
			m_outputConnectionPoints.clear();

			m_renderQueue = nullptr;
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
		void update(Echo::i32 xPos, Echo::i32 yPos)
		{
			m_rect->setPos(xPos * 200.f, yPos * 70.f);
		}
	};
	typedef Echo::vector<RenderQueueNodePainter*>::type RenderQueueNodePainters;
}

#endif