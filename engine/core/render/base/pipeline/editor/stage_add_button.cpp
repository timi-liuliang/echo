#include "stage_add_button.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/base/class_method_bind.h"
#include "engine/core/log/Log.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"
#include "stage_node_painter.h"

namespace Pipeline
{
	StatgeAddButton::StatgeAddButton(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderPipeline* pipeline)
	{
		m_pipeline = pipeline;
		m_graphicsView = view;
		m_graphicsScene = scene;

		initNextArrow();
		initStageDropRegion();
	}

	StatgeAddButton::~StatgeAddButton()
	{
		reset();
	}

	void StatgeAddButton::initStageDropRegion()
	{
		m_dropRegion = new QGraphicsDragDropRegionItem(nullptr);
		m_dropRegion->setZValue(-10.f);
		m_dropRegion->setAcceptDrops(true);
		m_dropRegion->setAcceptHoverEvents(true);
		m_dropRegion->setPos(QPointF(0.f, 0.f));
		m_dropRegion->setPen(QPen(m_stageDropRegionDefaultColor));
		m_dropRegion->setBrush(QBrush(m_stageDropRegionDefaultColor));
		m_graphicsScene->addItem(m_dropRegion);

		QPainterPath path;
		path.addRoundedRect(QRectF(StageNodePainter::getSpace() * 0.1f, 19.f, StageNodePainter::getSpace() * 0.8f - 2.f, m_graphicsScene->height() - 25.f), 0.f, 0.f);
		m_dropRegion->setPath(path);

		m_dropRegion->setDragEnterCb([this](QGraphicsSceneDragDropEvent* event)
		{
			return event->mimeData()->hasFormat("drag/render-stage") ? true : false;
		});

		m_dropRegion->setDragDropCb([this](QGraphicsSceneDragDropEvent* event)
		{
			if (event->mimeData()->hasFormat("drag/render-stage"))
			{
				using namespace Echo;

				Echo::i32 objectId = event->mimeData()->data("drag/render-stage").toInt();
				Echo::RenderStage* from = ECHO_DOWN_CAST<Echo::RenderStage*>(Echo::Object::getById(objectId));
				if (from)
				{
					vector<RenderStage*>::type& stages = m_pipeline->getRenderStages();
					for (size_t i = 0; i < stages.size(); i++)
					{
						if (stages[i] == from)
						{
							stages[i] = nullptr;
							stages.insert(stages.begin() + m_stagePosition, from);
							stages.erase(std::remove(stages.begin(), stages.end(), nullptr), stages.end());
							break;
						}
					}
				}
			}
		});
	}

	void StatgeAddButton::updateStageDropRegion()
	{
		float halfWidth = StageNodePainter::getHalfWidth();
		Echo::Vector2 stagePostion = Echo::Vector2(m_stagePosition * (StageNodePainter::getWidth() + StageNodePainter::getSpace()), 0.f);
		m_dropRegion->setPos(QPointF(stagePostion.x - halfWidth - 37.f, 0.f));

		QPainterPath path;
		path.addRoundedRect(QRectF(StageNodePainter::getSpace() * 0.1f, 19.f, StageNodePainter::getSpace() * 0.8f - 2.f, m_graphicsView->height() - 19.f - 21.f), 0.f, 0.f);
		m_dropRegion->setPath(path);

		if (m_dropRegion->isDropEnter())
		{
			m_dropRegion->setPen(QPen(m_stageDropRegionHighlightColor, 1.5f, Qt::DashLine));
			m_dropRegion->setBrush(QBrush(m_stageDropRegionDefaultColor));
		}
		else
		{
			m_dropRegion->setPen(QPen(m_stageDropRegionDefaultColor, 1.5f));
			m_dropRegion->setBrush(QBrush(m_stageDropRegionDefaultColor));
		}
	}

	void StatgeAddButton::initNextArrow()
	{
		QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/pipeline/editor/icon/right-arrow.png").c_str());
		m_nextArrow = new QGraphicsPixmapItemCustom();
		m_nextArrow->setPixmap(rightArrow.scaled(QSize(16, 16)));
		m_nextArrow->setAcceptHoverEvents(true);
		m_nextArrow->setToolTip("Add New RenderStage");
		m_graphicsScene->addItem(m_nextArrow);

		m_nextArrow->setHoverEnterEventCb([this](QGraphicsPixmapItem* item)
		{
			m_nextArrowHighlight = true;
		});

		m_nextArrow->setHoverEnterLeaveCb([this](QGraphicsPixmapItem* item)
		{
			m_nextArrowHighlight = false;
		});

		m_nextArrow->setMousePressEventCb([this](QGraphicsPixmapItem* item)
		{
			showAddStageMenu();
		});
	}

	void StatgeAddButton::updateNextArrow(bool isFinal)
	{
		Echo::String imagePath;
		if (isFinal)
		{
			imagePath = m_nextArrowHighlight ? "engine/core/render/base/pipeline/editor/icon/next-arrow-final.png":"engine/core/render/base/pipeline/editor/icon/next-arrow-dark-final.png";
		}
		else
		{
			imagePath = m_nextArrowHighlight ? "engine/core/render/base/pipeline/editor/icon/next-arrow.png":"engine/core/render/base/pipeline/editor/icon/next-arrow-dark.png";
		}

		QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + imagePath).c_str());
		m_nextArrow->setPixmap(rightArrow.scaled(QSize(36, 18), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

		float halfWidth = StageNodePainter::getHalfWidth();
		Echo::Vector2 stagePostion = Echo::Vector2(m_stagePosition * (StageNodePainter::getWidth() + StageNodePainter::getSpace()), 0.f);
		m_nextArrow->setPos(QPointF(stagePostion.x - halfWidth - 37.f, 0.f));
	}

	void StatgeAddButton::showAddStageMenu()
	{
		if (!m_addMenu)
		{
			m_addMenu = EchoNew(QMenu(m_graphicsView));

			Echo::StringArray childClasses = { "RenderStage"};
			Echo::Class::getChildClasses(childClasses, "RenderStage", true);
			for (const Echo::String& className : childClasses)
			{
				QAction* addRenderStageAction = new QAction(className.c_str());
				m_addMenu->addAction(addRenderStageAction);

				EditorApi.qConnectAction(addRenderStageAction, QSIGNAL(triggered()), this, Echo::createMethodBind(&StatgeAddButton::addNewStage));
			}
		}

		m_addMenu->exec(QCursor::pos());
	}

	void StatgeAddButton::addNewStage()
	{
		if (m_pipeline)
		{
			QAction* action = qobject_cast<QAction*>(EditorApi.qSender());
			if (action)
			{
				Echo::String text = action->text().toStdString().c_str();

				Echo::RenderStage* stage = dynamic_cast<Echo::RenderStage*>(Echo::Class::create(text));
				stage->setPipeline(m_pipeline);
				stage->setName("New Stage");

				m_pipeline->addStage(stage, m_stagePosition);
			}
		}
	}

	void StatgeAddButton::reset()
	{
		if (m_nextArrow)
			m_graphicsScene->removeItem(m_nextArrow);

		m_pipeline = nullptr;
		m_graphicsScene = nullptr;
		m_nextArrow = nullptr;
	}

	void StatgeAddButton::update(Echo::i32 xPos, bool isFinal)
	{
		m_stagePosition = xPos;
		updateNextArrow(isFinal);
		updateStageDropRegion();
	}
}

#endif