#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/core/main/Engine.h"
#include "custom/qgraphics_pixmap_item_custom.h"
#include "custom/qgraphics_renderstage_item.h"
#include "custom/qgraphics__drag_drop_region_item.h"

namespace Pipeline
{
	struct RenderQueueAddButton
	{
	public:
		RenderQueueAddButton(QGraphicsScene* scene, Echo::RenderPipeline* pipeline);
		~RenderQueueAddButton();

		// reset
		void reset();

		// update
		void update(Echo::i32 xPos, Echo::i32 yPos);

	private:
		// drop region
		void initDropRegion();
		void updateDropRegion();

	public:
		Echo::RenderPipeline*		m_pipeline = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		Echo::ui32					m_xPos;
		Echo::ui32					m_yPos;
		QColor						m_dropRegionDefaultColor = QColor(58, 58, 58, 1);
		QColor						m_dropRegionHighlightColor = QColor(255, 214, 98, 125); //Qt::green;
		QGraphicsDragDropRegionItem*m_dropRegion = nullptr;
	};
	typedef Echo::vector<RenderQueueAddButton*>::type RenderQueueAddButtons;
}

#endif