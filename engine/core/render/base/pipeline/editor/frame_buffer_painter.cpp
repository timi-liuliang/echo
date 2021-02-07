#include "frame_buffer_painter.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/main/Engine.h"
#include "stage_node_painter.h"

namespace Pipeline
{
	FrameBufferPainter::FrameBufferPainter(QGraphicsView* view, QGraphicsScene* scene)
	{
		m_graphicsView = view;
		m_graphicsScene = scene;
	}

	FrameBufferPainter::~FrameBufferPainter()
	{
		reset();

		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
	}

	void FrameBufferPainter::reset()
	{
		for (QGraphicsPathItem* rect : m_rects)
		{
			if (rect)
				m_graphicsScene->removeItem(rect);
		}

		for (QGraphicsPixmapItemCustom* image : m_images)
		{
			if (image)
				m_graphicsScene->removeItem(image);
		}

		m_rects.clear();
		m_images.clear();
	}

	void FrameBufferPainter::capture(Echo::FrameBuffer* fb)
	{
		reset();

		Echo::Vector2 pos = Echo::Vector2::ZERO;
		for (Echo::i32 i = Echo::FrameBuffer::Color0; i <= Echo::FrameBuffer::DepthStencil; i++)
		{
			Echo::FrameBuffer::Pixels pixels;
			if (fb->readPixels(Echo::FrameBuffer::Attachment(i), pixels))
			{
				float height = getHeight();
				float width = height / pixels.m_height * pixels.m_width;

				float halfWidth = width * 0.5f;
				float halfHeight = height * 0.5f;

				// bounding rect
				QPainterPath path;
				path.addRoundedRect(QRectF(-halfWidth - 1, -halfHeight - 1, width + 1, height + 1), 0.f, 0.f);

				QGraphicsPathItem* rectItem = createBoundRect(pos);
				rectItem->setPath(path);

				// pixmap item
				Echo::Image image(pixels.m_data.data(), pixels.m_width, pixels.m_height, 1, pixels.m_format);
				image.scale(width, height, Echo::Image::ImageFilter::IMGFILTER_LINEAR);
				image.convertFormat(Echo::PF_RGBA8_UNORM);

				QImage qimage(image.getData(), width, height, QImage::Format_RGBA8888);
				qimage = qimage.mirrored(false, true);
				qimage = qimage.convertToFormat(QImage::Format_RGB888);

				QGraphicsPixmapItemCustom* imageItem = createImage(rectItem);
				imageItem->setPixmap(QPixmap::fromImage(qimage));

				imageItem->setPos(QPointF(-halfWidth, -halfHeight));
				imageItem->setZValue(1.f);

				m_rects.emplace_back(rectItem);
				m_images.emplace_back(imageItem);

				pos.y += height + getSpace();
			}
		}
	}

	QGraphicsPathItem* FrameBufferPainter::createBoundRect(const Echo::Vector2& pos)
	{
		QColor normalBoundaryColor = QColor(66, 66, 66);
		float  penWidth = 1.f;

		QGraphicsPathItem* rect = new QGraphicsRenderQueueItem(nullptr, 0);
		rect->setPen(QPen(normalBoundaryColor, penWidth));
		rect->setFlag(QGraphicsItem::ItemIsFocusable);
		rect->setAcceptHoverEvents(true);
		rect->setFiltersChildEvents(true);
		rect->setPos(QPointF(pos.x, pos.y));
		m_graphicsScene->addItem(rect);

		return rect;
	}

	QGraphicsPixmapItemCustom* FrameBufferPainter::createImage(QGraphicsPathItem* rect)
	{
		QGraphicsPixmapItemCustom* image = new QGraphicsPixmapItemCustom();
		image->setParentItem(rect);
		m_graphicsScene->addItem(image);

		return image;
	}
}

#endif