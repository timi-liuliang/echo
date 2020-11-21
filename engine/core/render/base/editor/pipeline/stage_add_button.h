#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "engine/core/main/Engine.h"
#include "custom/qgraphics_pixmap_item_custom.h"
#include "custom/qgraphics_renderstage_item.h"
#include "custom/qgraphics__drag_drop_region_item.h"

namespace Pipeline
{
	struct StatgeAddButton
	{
	public:
		StatgeAddButton(QGraphicsScene* scene, Echo::RenderPipeline* pipeline);
		~StatgeAddButton();

		// reset
		void reset();

		// update
		void update(Echo::i32 xPos, bool isFinal);

		// on add
		void showAddStageMenu();

		// add new stage
		void addNewStage();

	private:
		// next arrow
		void initNextArrow();
		void updateNextArrow(bool isFinal);

		// drop region
		void initStageDropRegion();
		void updateStageDropRegion();

	public:
		Echo::RenderPipeline*		m_pipeline = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		Echo::ui32					m_stagePosition;
		QGraphicsPixmapItemCustom*	m_nextArrow = nullptr;
		bool						m_nextArrowHighlight = false;
		QColor						m_stageDropRegionDefaultColor = QColor(83, 83, 83);
		QColor						m_stageDropRegionHighlightColor = QColor(255, 214, 98, 125); //Qt::green;
		QGraphicsDragDropRegionItem*m_stageDropRegion = nullptr;
	};
	typedef Echo::vector<StatgeAddButton*>::type StatgeAddButtons;
}

#endif