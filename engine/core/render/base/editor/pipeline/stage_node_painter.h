#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_stage.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "engine/core/main/Engine.h"
#include "custom/qgraphics_pixmap_item_custom.h"
#include "custom/qgraphics_renderstage_item.h"
#include "renderqueue_node_painter.h"
#include "renderqueue_add_button.h"

namespace Pipeline
{
	struct StageNodePainter
	{
	public:
		// Style for painting
		struct Style
		{
			QColor m_normalBoundaryColor = QColor(137, 137, 137);
			QColor m_selectedBoundaryColor = QColor(255, 165, 0);
			QColor m_gradientColor0 = Qt::gray;
			QColor m_gradientColor1 = QColor(80, 80, 80);
			QColor m_gradientColor2 = QColor(64, 64, 64);
			QColor m_gradientColor3 = QColor(58, 58, 58);
			QColor m_shadowColor = QColor(20, 20, 20);
			QColor m_fontColor = Qt::gray;
			QColor m_fontColorFaded = Qt::gray;
			QColor m_warningColor = QColor(128, 128, 0);
			QColor m_errorColor = Qt::red;
			QColor finalColor = QColor(54, 108, 179, 255);
			float  m_penWidth = 1.5f;
			float  m_cornerRadius = 0.f;
		};

	public:
		StageNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::RenderStage* stage);
		~StageNodePainter();

		// reset
		void reset();

		// update
		void update(Echo::i32 xPos, bool isFinal);

	public:
		// get width
		static float getWidth() { return 190.f; }
		static float getHalfWidth() { return getWidth() * 0.5f; }
		static float getSpace() { return 38.f; }

	public:
		// new
		void onNewImageFilter();
		void onNewRenderQueue();

		// on add
		void showAddQueueMenu();

		// delete
		void onDeleteThisRenderStage();

	private:
		// boundary
		void initBoundary();

		// render queues
		void updateRenderQueues(Echo::i32 xPos);
		void updateRenderQueueAddButtons(Echo::i32 xPos);

	public:
		Style						m_style;
		Echo::RenderStage*			m_stage = nullptr;
		QGraphicsView*				m_graphicsView = nullptr;
		QGraphicsScene*				m_graphicsScene = nullptr;
		QGraphicsRenderStageItem*	m_rect = nullptr;
		QGraphicsPixmapItemCustom*  m_addAction = nullptr;
		size_t						m_renderQueueSize = -1;
		bool						m_enable = true;
		float						m_height = 240;
		QGraphicsSimpleTextItem*	m_text = nullptr;
		QMenu*						m_addMenu = nullptr;
		RenderQueueNodePainters		m_renderQueueNodePainters;
		RenderQueueAddButtons		m_renderQueueAddButtons;
	};
	typedef Echo::vector<StageNodePainter*>::type StageNodePainters;
}

#endif