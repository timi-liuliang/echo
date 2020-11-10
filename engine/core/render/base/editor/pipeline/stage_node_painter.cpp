#include "stage_node_painter.h"
#include "engine/core/base/class_method_bind.h"

#ifdef ECHO_EDITOR_MODE

namespace Pipeline
{
	StatgeNodePainter::StatgeNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderStage* stage)
	{
		m_stage = stage;
		m_graphicsView = view;
		m_graphicsScene = scene;

		if (!m_rect)
		{
			float halfWidth = m_width * 0.5f;

			m_rect = new QGraphicsRenderStageItem(nullptr);
			m_rect->setZValue(-1.f);
			m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
			m_rect->setFlag(QGraphicsItem::ItemIsFocusable, true);
			m_rect->setPos(QPointF(0.f, 0.f));
			m_graphicsScene->addItem(m_rect);

			// mouse press event
			m_rect->setMousePressEventCb([this](QGraphicsItem* item)
			{
				EditorApi.showObjectProperty(m_stage);
			});

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

			m_addAction->setMousePressEventCb([this](QGraphicsPixmapItem* item)
			{
				showAddMenu();
			});
		}
	}

	StatgeNodePainter::~StatgeNodePainter()
	{
		reset();
	}

	void StatgeNodePainter::showAddMenu()
	{
		if (!m_addMenu)
		{
			m_addMenu = EchoNew(QMenu(nullptr));

			QAction* newImageFilter = new QAction("Image Filter");
			QAction* newRenderQueue = new QAction("Render Queue");

			m_addMenu->addAction(newImageFilter);
			m_addMenu->addAction(newRenderQueue);

			EditorApi.qConnectAction(newImageFilter, QSIGNAL(triggered()), this, Echo::createMethodBind(&StatgeNodePainter::onNewImageFilter));
			EditorApi.qConnectAction(newRenderQueue, QSIGNAL(triggered()), this, Echo::createMethodBind(&StatgeNodePainter::onNewRenderQueue));
		}

		m_addMenu->exec(QCursor::pos());
	}

	void StatgeNodePainter::onNewImageFilter()
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

	void StatgeNodePainter::onNewRenderQueue()
	{

	}

	void StatgeNodePainter::reset()
	{
		if (m_rect)
			m_graphicsScene->removeItem(m_rect);

		m_stage = nullptr;
		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
		m_rect = nullptr;
		m_text = nullptr;
	}

	void StatgeNodePainter::update(Echo::i32 xPos, bool isFinal)
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

			m_text->setText(m_stage->getName().c_str());

			m_rect->setPos(xPos * 240.f, 0.f);
			m_rect->setPen(QPen(m_rect->isFocused() ? m_style.m_selectedBoundaryColor : m_style.m_normalBoundaryColor, m_style.m_penWidth));

			m_nextArrow->setVisible(!isFinal);
		}
	}
}

#endif