#pragma once

#include "engine/core/editor/editor.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/render/base/pipeline/render_pass.h"
#include "engine/modules/procedural/procedural_geometry.h"
#include "custom/qgraphics_renderqueue_item.h"
#include "custom/qgraphics_pixmap_item_custom.h"

namespace Pipeline
{
	class RenderQueueNodePainter
	{
	public:
		RenderQueueNodePainter(QGraphicsView* view, QGraphicsScene* scene, Echo::IRenderQueue* queue);
		~RenderQueueNodePainter() { reset(); }

		// get render queue
		Echo::IRenderQueue* getRenderQueue() { return m_renderQueue; }

		// reset
		void reset();

		// update
		void update(Echo::i32 xPos, Echo::i32 yPos);

	public:
		// start pos
		static float getStartPos() { return 60.f; }

		// get height|width
		static float getWidth() { return 160.f; }
		static float getHeight(){ return 40.f; }
		static float getSpace() { return 16.f; }

	protected:
		// init delete action
		void initDeleteButton();

		// delete
		void deleteThisRenderQueue();

	protected:
		struct
		{
			QColor m_normalBoundaryColor = QColor(137, 137, 137);
			QColor m_disableBoundaryColor = QColor(77, 77, 77);
			QColor m_selectedBoundaryColor = QColor(255, 165, 0);
			QColor m_gradientColor0 = Qt::gray;
			QColor m_gradientColor1 = QColor(80, 80, 80);
			QColor m_gradientColor2 = QColor(64, 64, 64);
			QColor m_gradientColor3 = QColor(58, 58, 58);
			QColor m_shadowColor = QColor(20, 20, 20);
			QColor m_fontColor = Qt::gray;
			QColor m_fontColorFaded = Qt::darkGray;
			QColor m_connectionPointColor = QColor(169, 169, 169);
			QColor m_filledConnectionPointColor = Qt::cyan;
			QColor m_warningColor = QColor(128, 128, 0);
			QColor m_errorColor = Qt::red;
			QColor finalColor = QColor(54, 108, 179, 255);
			float  m_penWidth = 1.5f;
			float  m_cornerRadius = 0.f;
		}									m_style;
		Echo::IRenderQueue*					m_renderQueue = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		QGraphicsScene*						m_graphicsScene = nullptr;
		QGraphicsRenderQueueItem*			m_rect = nullptr;
		float								m_rectFinalWidth = 15;
		QGraphicsSimpleTextItem*			m_text = nullptr;
		QGraphicsLineItem*					m_textDiableLine = nullptr;
		QGraphicsPixmapItemCustom*			m_deleteButtton = nullptr;
	};
	typedef Echo::vector<RenderQueueNodePainter*>::type RenderQueueNodePainters;
}

#endif