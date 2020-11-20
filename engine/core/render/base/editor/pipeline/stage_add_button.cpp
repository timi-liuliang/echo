#include "stage_add_button.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/log/Log.h"
#include "engine/core/render/base/pipeline/render_pipeline.h"

#ifdef ECHO_EDITOR_MODE

namespace Pipeline
{
	StatgeAddButton::StatgeAddButton(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderStage* stage)
	{
		m_stage = stage;
		m_graphicsView = view;
		m_graphicsScene = scene;

		if (!m_rect)
		{
			initBoundary();

			initNextArrow();
		}
	}

	StatgeAddButton::~StatgeAddButton()
	{
		reset();
	}

	void StatgeAddButton::initBoundary()
	{
		float halfWidth = m_width * 0.5f;

		m_rect = new QGraphicsRenderStageItem(nullptr);
		m_rect->setZValue(-1.f);
		m_rect->setPen(QPen());
		m_rect->setFlag(QGraphicsItem::ItemIsFocusable, true);
		m_rect->setPos(QPointF(0.f, 0.f));
		m_graphicsScene->addItem(m_rect);

		// mouse press event
		m_rect->setMousePressEventCb([this](QGraphicsItem* item)
		{
			EditorApi.showObjectProperty(m_stage);
		});

		m_rect->setKeyPressEventCb([this](QKeyEvent* event)
		{
			if (event->key() == Qt::Key_Delete)
				onDeleteThisRenderStage();
		});
	}

	void StatgeAddButton::initNextArrow()
	{
		float halfWidth = m_width * 0.5f;

		QPixmap rightArrow((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/right-arrow.png").c_str());
		m_nextArrow = new QGraphicsPixmapItemCustom();
		m_nextArrow->setPixmap(rightArrow.scaled(QSize(16, 16)));
		m_nextArrow->setParentItem(m_rect);
		m_nextArrow->setPos(QPointF(-halfWidth - 37.f, 0.f));
		m_nextArrow->setAcceptHoverEvents(true);
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
	}

	void StatgeAddButton::showAddStageMenu()
	{
		addNewStage();
	}

	void StatgeAddButton::addNewStage()
	{
		Echo::RenderPipeline* pipeline = m_stage ? m_stage->getPipeline() : nullptr;
		if (pipeline)
		{
			Echo::RenderStage* stage = EchoNew(Echo::RenderStage(pipeline));
			stage->setName("New Stage");

			pipeline->addStage(stage);
		}
	}

	void StatgeAddButton::onNewImageFilter()
	{
		if (m_stage)
		{
			Echo::ImageFilter* imageFilter = m_stage->addImageFilter("New Image Filter");
			if (imageFilter)
			{
				EditorApi.showObjectProperty(imageFilter);
			}
		}
	}

	void StatgeAddButton::onDeleteThisRenderStage()
	{
		Echo::RenderPipeline* pipeline = m_stage ? m_stage->getPipeline() : nullptr;
		if (pipeline)
		{
			if (QMessageBox::Yes == QMessageBox(QMessageBox::Warning, "Warning", "Do you really want to delete the selected render stage ?", QMessageBox::Yes | QMessageBox::No).exec())
			{
				EditorApi.showObjectProperty(pipeline);
				pipeline->deleteStage(m_stage);
			}
		}
	}

	void StatgeAddButton::onNewRenderQueue()
	{
	}

	void StatgeAddButton::reset()
	{
		if (m_rect)
			m_graphicsScene->removeItem(m_rect);

		m_stage = nullptr;
		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
		m_rect = nullptr;
	}

	void StatgeAddButton::update(Echo::i32 xPos, bool isFinal)
	{
		if (m_rect)
		{
			m_rect->setPos(xPos * 228.f, 0.f);
			updateNextArrow(isFinal);
		}
	}
}

#endif