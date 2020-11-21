#include "stage_add_button.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/log/Log.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

namespace Pipeline
{
	StatgeAddButton::StatgeAddButton(QGraphicsScene* scene, Echo::RenderPipeline* pipeline)
	{
		m_pipeline = pipeline;
		m_graphicsScene = scene;

		initNextArrow();
	}

	StatgeAddButton::~StatgeAddButton()
	{
		reset();
	}

	void StatgeAddButton::initNextArrow()
	{
		QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/right-arrow.png").c_str());
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
			imagePath = m_nextArrowHighlight ? "engine/core/render/base/editor/icon/next-arrow-final.png":"engine/core/render/base/editor/icon/next-arrow-dark-final.png";
		}
		else
		{
			imagePath = m_nextArrowHighlight ? "engine/core/render/base/editor/icon/next-arrow.png":"engine/core/render/base/editor/icon/next-arrow-dark.png";
		}

		QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + imagePath).c_str());
		m_nextArrow->setPixmap(rightArrow.scaled(QSize(36, 18), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

		float halfWidth = m_stageWidth * 0.5f;
		m_nextArrow->setPos(QPointF(m_stagePosition.x - halfWidth - 37.f, 0.f));
	}

	void StatgeAddButton::showAddStageMenu()
	{
		addNewStage();
	}

	void StatgeAddButton::addNewStage()
	{
		if (m_pipeline)
		{
			Echo::RenderStage* stage = EchoNew(Echo::RenderStage(m_pipeline));
			stage->setName("New Stage");

			m_pipeline->addStage(stage);
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
		m_stagePosition = Echo::Vector2(xPos * 228.f, 0.f);
		updateNextArrow(isFinal);
	}
}

#endif