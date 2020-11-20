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
		StatgeAddButton(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderStage* stage);
		~StatgeAddButton();

		// reset
		void reset();

		// update
		void update(Echo::i32 xPos, bool isFinal);

		// on add
		void showAddStageMenu();

		// add new stage
		void addNewStage();

	public:
		// new
		void onNewImageFilter();
		void onNewRenderQueue();

		// delete
		void onDeleteThisRenderStage();

	private:
		// boundary
		void initBoundary();

		// next arrow
		void initNextArrow();
		void updateNextArrow(bool isFinal);

	public:
		Echo::RenderStage*			m_stage = nullptr;
		QGraphicsView*				m_graphicsView = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		QGraphicsRenderStageItem*	m_rect = nullptr;
		QGraphicsPixmapItemCustom*	m_nextArrow = nullptr;
		bool						m_nextArrowHighlight = false;
		float						m_width = 190;
		float						m_height = 240;
	};
	typedef Echo::vector<StatgeAddButton*>::type StatgeAddButtons;
}

#endif