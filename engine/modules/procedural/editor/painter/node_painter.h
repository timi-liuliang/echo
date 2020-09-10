#pragma once

#include "engine/core/editor/editor.h"
#include "engine/modules/procedural/procedural_geometry.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	struct PGNodesPainter
	{
		QWidget*		m_graphicsView = nullptr;
		QObject*		m_graphicsScene = nullptr;
		QGraphicsItem*	m_rect = nullptr;

		~PGNodesPainter()
		{
		}

		// set
		void set(QWidget* view, QObject* scene)
		{
			m_graphicsView = view;
			m_graphicsScene = scene;
		}

		// reset
		void reset()
		{
			if (m_rect)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_rect);
				m_rect = nullptr;
			}
		}

		// update
		void update(Echo::ProceduralGeometry* pg)
		{
			if (!m_rect)
			{
				m_rect = EditorApi.qGraphicsSceneAddRect(m_graphicsScene, 0.f, 0.f, 200.f, 100.f, Echo::Color::RED);
			}

			for (Echo::PGNode* pgNode : pg->getPGNodes())
			{

			}
		}
	};
}

#endif