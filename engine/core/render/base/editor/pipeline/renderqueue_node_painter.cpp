#include "renderqueue_node_painter.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/main/Engine.h"
#include "stage_node_painter.h"

namespace Pipeline
{
	RenderQueueNodePainter::RenderQueueNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::IRenderQueue* queue)
	{
		m_renderQueue = queue;
		m_graphicsView = view;
		m_graphicsScene = scene;

		m_renderQueue->onRenderEnd.connectClassMethod(this, Echo::createMethodBind(&RenderQueueNodePainter::onCaptureFrame));

		if (!m_rect)
		{
			float halfWidth = getWidth() * 0.5f;
			float halfHeight = getHeight() * 0.5f;

			QPainterPath path;
			path.addRoundedRect(QRectF(-halfWidth, -halfHeight, getWidth(), getHeight()), m_style.m_cornerRadius, m_style.m_cornerRadius);

			m_rect = new QGraphicsRenderQueueItem(nullptr, queue->getId());
			m_rect->setPath(path);
			m_rect->setPen(QPen(m_style.m_normalBoundaryColor, m_style.m_penWidth));
			m_rect->setFlag(QGraphicsItem::ItemIsFocusable);
			m_rect->setAcceptHoverEvents(true);
			m_rect->setFiltersChildEvents(true);
			QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
			gradient.setColorAt(0.0, m_style.m_gradientColor0);
			gradient.setColorAt(0.03, m_style.m_gradientColor1);
			gradient.setColorAt(0.97, m_style.m_gradientColor2);
			gradient.setColorAt(1.0, m_style.m_gradientColor3);
			m_rect->setBrush(gradient);
			m_rect->setPos(QPointF(0.f, 0.f));
			m_graphicsScene->addItem(m_rect);

			// mouse press event
			m_rect->setMousePressEventCb([this](QGraphicsItem* item)
			{
				EditorApi.showObjectProperty(m_renderQueue);
				m_needCaptureFrame = true;
			});

			m_rect->setKeyPressEventCb([this](QKeyEvent* event) 
			{
				if(event->key()==Qt::Key_Delete)
					deleteThisRenderQueue();
			});

			m_text = new QGraphicsSimpleTextItemCustom();
			m_text->setBrush(QBrush(m_style.m_fontColor));
			m_text->setParentItem(m_rect);
			m_graphicsScene->addItem(m_text);

			// mouse press event
			m_text->setMousePressEventCb([this](QGraphicsSimpleTextItem* item)
			{
				EditorApi.showObjectProperty(m_renderQueue);
				m_needCaptureFrame = true;
			});

			m_text->setKeyPressEventCb([this](QKeyEvent* event)
			{
				if (event->key() == Qt::Key_Delete)
					deleteThisRenderQueue();
			});

			m_textDiableLine = m_graphicsScene->addLine(QLineF(), QPen(m_style.m_fontColorFaded));
			m_textDiableLine->setVisible(false);
			m_textDiableLine->setParentItem(m_rect);

			// close
			initTypeButton();
			initDeleteButton();
		}
	}

	void RenderQueueNodePainter::initTypeButton()
	{
		bool isImageFilter = dynamic_cast<Echo::ImageFilter*>(m_renderQueue) ? true : false;
		Echo::String iconPath = isImageFilter ? "image_filter.png" : "render_queue.png";

		QPixmap icon((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/" + iconPath).c_str());
		m_typeButton = new QGraphicsPixmapItemCustom();
		m_typeButton->setPixmap(icon.scaled(QSize(16, 16)));
		m_typeButton->setParentItem(m_rect);
		m_typeButton->setPos(QPointF(- getWidth() * 0.5f + 12.f, -8.f));
		m_graphicsScene->addItem(m_typeButton);
	}

	void RenderQueueNodePainter::initDeleteButton()
	{
		QPixmap icon((Echo::Engine::instance()->getRootPath() + "engine/core/render/base/editor/icon/delete.png").c_str());
		m_deleteButtton = new QGraphicsPixmapItemCustom();
		m_deleteButtton->setPixmap(icon.scaled(QSize(16, 16)));
		m_deleteButtton->setParentItem(m_rect);
		m_deleteButtton->setPos(QPointF(getWidth() * 0.5f - 24.f, -8.f));
		m_deleteButtton->setAcceptHoverEvents(true);
		m_deleteButtton->setVisible(false);
		m_graphicsScene->addItem(m_deleteButtton);

		m_deleteButtton->setMousePressEventCb([this](QGraphicsPixmapItem* item)
		{
			deleteThisRenderQueue();
		});
	}

	void RenderQueueNodePainter::deleteThisRenderQueue()
	{
		if (m_renderQueue)
		{
			if (QMessageBox::Yes == QMessageBox(QMessageBox::Warning, "Warning", "Do you really want to delete the selected render queue ?", QMessageBox::Yes | QMessageBox::No).exec())
			{
				EditorApi.showObjectProperty(m_renderQueue->getStage());
				m_renderQueue->getStage()->deleteRenderQueue(m_renderQueue);
			}
		}
	}

	void RenderQueueNodePainter::reset()
	{
		m_renderQueue->onRenderEnd.disconnectAll();

		if (m_rect)
			m_graphicsScene->removeItem(m_rect);

		m_renderQueue = nullptr;
		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
		m_rect = nullptr;
		m_text = nullptr;
	}

	void RenderQueueNodePainter::update(Echo::i32 xPos, Echo::i32 yPos)
	{
		bool enable = m_renderQueue->isEnable() && m_renderQueue->getStage()->isEnable();

		float halfWidth = getWidth() * 0.5f;
		float halfHeight = getHeight() * 0.5f;

		float startYPos = getStartPos();
		float penWidth = enable ? m_style.m_penWidth : m_style.m_penWidth - 1;
		m_rect->setPen(QPen(m_rect->isFocused() ? m_style.m_selectedBoundaryColor : (m_renderQueue->isEnable() ? m_style.m_normalBoundaryColor : m_style.m_disableBoundaryColor), penWidth));
		m_rect->setPos(xPos * (StageNodePainter::getWidth() + StageNodePainter::getSpace()), startYPos + yPos * (getHeight()+getSpace()));

		m_deleteButtton->setVisible(m_rect->isFocused());

		if (enable)
		{
			QLinearGradient gradient(QPointF(0.0, -halfHeight), QPointF(0.0, halfHeight));
			gradient.setColorAt(0.0, m_style.m_gradientColor0);
			gradient.setColorAt(0.03, m_style.m_gradientColor1);
			gradient.setColorAt(0.97, m_style.m_gradientColor2);
			gradient.setColorAt(1.0, m_style.m_gradientColor3);
			m_rect->setBrush(gradient);
		}
		else
		{
			m_rect->setBrush(QBrush(m_style.m_gradientColor1));
		}

		m_text->setText(m_renderQueue->getName().c_str());
		m_text->setBrush(m_renderQueue->isEnable() ? QBrush(m_style.m_fontColor) : QBrush(m_style.m_fontColorFaded));

		QRectF textRect = m_text->sceneBoundingRect();
		m_text->setPos((getWidth() - textRect.width()) * 0.5f - halfWidth, (getHeight() - textRect.height()) * 0.5f - halfHeight);

		m_textDiableLine->setVisible(!enable);
		m_textDiableLine->setLine(-textRect.width() * 0.5f - 2.f, 0.f, textRect.width() * 0.5f + 2.f, 0.f);
		m_textDiableLine->setPos(0.f, 0.f);

		m_typeButton->setVisible(false);
	}

	void RenderQueueNodePainter::onCaptureFrame()
	{
		if (m_needCaptureFrame)
		{
			Echo::RenderStage* stage = m_renderQueue->getStage();
			if (stage)
			{
				Echo::FrameBuffer* fb = stage->getFrameBuffer();
				if (fb)
				{
					Echo::FrameBuffer::Pixels pixels;
					if (fb->readPixels(Echo::FrameBuffer::Attachment::Color0, pixels))
					{
						Echo::Image image(pixels.m_data.data(), pixels.m_width, pixels.m_height, 1, pixels.m_format);
						image.saveToFile("D:/test.bmp");
					}
				}
			}

			m_needCaptureFrame = false;
		}
	}
}

#endif