#include "renderqueue_add_button.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/base/class_method_bind.h"
#include "engine/core/log/Log.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"
#include "stage_node_painter.h"

namespace Pipeline
{
	RenderQueueAddButton::RenderQueueAddButton(QGraphicsScene* scene, Echo::RenderPipeline* pipeline)
	{
		m_pipeline = pipeline;
		m_graphicsScene = scene;

		initDropRegion();
	}

	RenderQueueAddButton::~RenderQueueAddButton()
	{
		reset();
	}

	void RenderQueueAddButton::initDropRegion()
	{
		m_dropRegion = new QGraphicsDragDropRegionItem(nullptr);
		m_dropRegion->setZValue(10.f);
		m_dropRegion->setAcceptDrops(true);
		m_dropRegion->setAcceptHoverEvents(true);
		m_dropRegion->setPos(QPointF(0.f, 0.f));
		m_dropRegion->setPen(QPen(m_dropRegionDefaultColor));
		m_dropRegion->setBrush(QBrush(m_dropRegionDefaultColor));
		m_graphicsScene->addItem(m_dropRegion);

		QPainterPath path;
		path.addRoundedRect(QRectF(-RenderQueueNodePainter::getWidth() * 0.5f, -RenderQueueNodePainter::getSpace() * 0.3f, RenderQueueNodePainter::getWidth(), RenderQueueNodePainter::getSpace() * 0.6f), 0.f, 0.f);
		m_dropRegion->setPath(path);

		m_dropRegion->setDragEnterCb([this](QGraphicsSceneDragDropEvent* event)
		{
			return event->mimeData()->hasFormat("drag/render-queue") ? true : false;
		});

		m_dropRegion->setDragDropCb([this](QGraphicsSceneDragDropEvent* event)
		{
			if (event->mimeData()->hasFormat("drag/render-queue"))
			{
				Echo::i32 objectId = event->mimeData()->data("drag/render-queue").toInt();
				Echo::IRenderQueue* from = ECHO_DOWN_CAST<Echo::IRenderQueue*>(Echo::Object::getById(objectId));
				if (from)
				{

				}
			}
		});
	}

	void RenderQueueAddButton::updateDropRegion()
	{
		float queueNodeHalfHeight = (RenderQueueNodePainter::getSpace() + RenderQueueNodePainter::getHeight()) * 0.5f;
		float yPosition = m_yPos * (RenderQueueNodePainter::getHeight() + RenderQueueNodePainter::getSpace()) + RenderQueueNodePainter::getStartPos() - queueNodeHalfHeight;
		Echo::Vector2 stagePostion = Echo::Vector2(m_xPos * (StageNodePainter::getWidth() + StageNodePainter::getSpace()), yPosition);
		m_dropRegion->setPos(QPointF(stagePostion.x, stagePostion.y));

		if (m_dropRegion->isDropEnter())
		{
			m_dropRegion->setPen(QPen(m_dropRegionHighlightColor, 1.5f, Qt::DashLine));
			m_dropRegion->setBrush(QBrush(m_dropRegionDefaultColor));
		}
		else
		{
			m_dropRegion->setPen(QPen(m_dropRegionDefaultColor, 1.5f));
			m_dropRegion->setBrush(QBrush(m_dropRegionDefaultColor));
		}
	}

	void RenderQueueAddButton::reset()
	{
		m_pipeline = nullptr;
		m_graphicsScene = nullptr;
	}

	void RenderQueueAddButton::update(Echo::i32 xPos, Echo::i32 yPos)
	{
		m_xPos = xPos;
		m_yPos = yPos;
		updateDropRegion();
	}
}

#endif