#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "engine/core/main/Engine.h"
#include "custom/qgraphics_pixmap_item_custom.h"
#include "custom/qgraphics_renderstage_item.h"

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

	public:
		Echo::RenderPipeline*		m_pipeline = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		Echo::Vector2				m_stagePosition;
		QGraphicsPixmapItemCustom*	m_nextArrow = nullptr;
		bool						m_nextArrowHighlight = false;
		float						m_stageWidth = 190;
	};
	typedef Echo::vector<StatgeAddButton*>::type StatgeAddButtons;
}

#endif