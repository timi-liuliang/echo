#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "custom/qgraphics_renderqueue_item.h"
#include "custom/qgraphics_pixmap_item_custom.h"
#include "custom/qgraphics_text_item_custom.h"

namespace Pipeline
{
	class FrameBufferPainter
	{
	public:
		FrameBufferPainter(QGraphicsView* view, QGraphicsScene* scene);
		~FrameBufferPainter();

		// reset
		void capture(Echo::FrameBuffer* fb);

		// reset
		void reset();

	public:
		// start pos
		static float getHeight() { return 100.f; }

		// get height|width
		static float getSpace() { return 16.f; }

	protected:
		// create
		QGraphicsPathItem* createBoundRect(const Echo::Vector2& pos);
		QGraphicsPixmapItemCustom* createImage(QGraphicsPathItem* rect);

	protected:
		QGraphicsView*							m_graphicsView = nullptr;
		QGraphicsScene*							m_graphicsScene = nullptr;
		float									m_rectFinalWidth = 15;
		std::vector<QGraphicsPathItem*>			m_rects;
		std::vector<QGraphicsPixmapItemCustom*>	m_images;
	};
}

#endif