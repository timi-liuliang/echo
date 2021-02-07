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

		if (!m_rect)
		{
			QColor normalBoundaryColor = QColor(66, 66, 66);
			float  penWidth = 1.f;

			m_rect = new QGraphicsRenderQueueItem(nullptr, 0);
			m_rect->setPen(QPen(normalBoundaryColor, penWidth));
			m_rect->setFlag(QGraphicsItem::ItemIsFocusable);
			m_rect->setAcceptHoverEvents(true);
			m_rect->setFiltersChildEvents(true);
			m_rect->setPos(QPointF(0.f, 0.f));
			m_graphicsScene->addItem(m_rect);

			// close
			initImage();
		}
	}

	FrameBufferPainter::~FrameBufferPainter()
	{
		if (m_rect)
			m_graphicsScene->removeItem(m_rect);

		m_graphicsView = nullptr;
		m_graphicsScene = nullptr;
		m_rect = nullptr;
	}

	void FrameBufferPainter::capture(Echo::FrameBuffer* fb)
	{
		Echo::FrameBuffer::Pixels pixels;
		if (fb->readPixels(Echo::FrameBuffer::Attachment::Color0, pixels))
		{
			float height = 100.f;
			float width = height / pixels.m_height * pixels.m_width;

			Echo::Image image(pixels.m_data.data(), pixels.m_width, pixels.m_height, 1, pixels.m_format);
			image.scale(width, height, Echo::Image::ImageFilter::IMGFILTER_NEAREST);

			QImage qimage(image.getData(), width, height, QImage::Format_RGBA8888);
			qimage = qimage.mirrored(false, true);
			qimage = qimage.convertToFormat(QImage::Format_RGB888);

			m_image->setPixmap(QPixmap::fromImage(qimage));

			float halfWidth = width * 0.5f;
			float halfHeight = height * 0.5f;

			QPainterPath path;
			path.addRoundedRect(QRectF(-halfWidth-1, -halfHeight-1, width+1, height+1), 0.f, 0.f);
			m_rect->setPath(path);

			m_image->setPos(QPointF(-halfWidth, -halfHeight));
			m_image->setZValue(1.f);
		}
	}

	void FrameBufferPainter::initImage()
	{
		m_image = new QGraphicsPixmapItemCustom();
		m_image->setParentItem(m_rect);
		m_graphicsScene->addItem(m_image);
	}
}

#endif