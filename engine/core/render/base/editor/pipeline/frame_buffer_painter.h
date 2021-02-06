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

	public:
		// start pos
		static float getStartPos() { return 60.f; }

		// get height|width
		static float getSpace() { return 16.f; }

	protected:
		// init icons
		void initImage();

	protected:
		QGraphicsView*							m_graphicsView = nullptr;
		QGraphicsScene*							m_graphicsScene = nullptr;
		QGraphicsRenderQueueItem*				m_rect = nullptr;
		float									m_rectFinalWidth = 15;
		QGraphicsPixmapItemCustom*				m_image = nullptr;
	};
}

#endif