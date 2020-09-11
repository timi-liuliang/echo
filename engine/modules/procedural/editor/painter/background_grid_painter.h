#pragma once

#include "engine/core/editor/qt/QWidget.h"
#include "engine/core/editor/editor.h"
#include "engine/core/math/Rect.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	struct QBackgroundGridPainter
	{
		QWidget*		m_graphicsView = nullptr;
		QObject*		m_graphicsScene = nullptr;
		QGraphicsItem*	m_horizonalLines = nullptr;
		QGraphicsItem*	m_verticalLines = nullptr;
		Echo::Color		m_color;
		Echo::Rect		m_viewRect;
		Echo::Real		m_gridStep;

		~QBackgroundGridPainter()
		{
			reset();
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
			if (m_horizonalLines)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_horizonalLines);
				m_horizonalLines = nullptr;
			}

			if (m_verticalLines)
			{
				EditorApi.qGraphicsSceneDeleteItem(m_graphicsScene, m_verticalLines);
				m_verticalLines = nullptr;
			}
		}

		// update
		void update(Echo::Real gridStep, const Echo::Color& color)
		{
			if (m_graphicsView && m_graphicsScene)
			{
				Echo::Rect viewRect;
				EditorApi.qGraphicsViewSceneRect(m_graphicsView, viewRect);

				if (m_gridStep != gridStep || m_color != color || m_viewRect != viewRect)
				{
					reset();

					Echo::Real left = std::floor(viewRect.left / gridStep - 1.0);
					Echo::Real right = std::floor(viewRect.right / gridStep + 1.0);
					Echo::Real top = std::floor(viewRect.top / gridStep - 1.0);
					Echo::Real bottom = std::floor(viewRect.bottom / gridStep + 1.0);

					// vertical lines
					Echo::vector<Echo::Vector2>::type vpaths;
					for (int xi = int(left); xi <= int(right); ++xi)
					{
						if (xi % 2 == 0)
						{
							// top -> bottom
							vpaths.emplace_back(xi * gridStep, top * gridStep);
							vpaths.emplace_back(xi * gridStep, bottom * gridStep);
						}
						else
						{
							// bottom -> top
							vpaths.emplace_back(Echo::Vector2(xi * gridStep, bottom * gridStep));
							vpaths.emplace_back(Echo::Vector2(xi * gridStep, top * gridStep));
						}
					}

					// horizontal lines
					Echo::vector<Echo::Vector2>::type hpaths;
					for (int yi = int(top); yi <= int(bottom); ++yi)
					{
						if (yi % 2 == 0)
						{
							// left -> right
							hpaths.emplace_back(left * gridStep, yi * gridStep);
							hpaths.emplace_back(right * gridStep, yi * gridStep);
						}
						else
						{
							// right -> left
							hpaths.emplace_back(right * gridStep, yi * gridStep);
							hpaths.emplace_back(left * gridStep, yi * gridStep);
						}
					}

					m_verticalLines = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, vpaths, 1.f, m_color);
					m_horizonalLines = EditorApi.qGraphicsSceneAddPath(m_graphicsScene, hpaths, 1.f, m_color);

					EditorApi.qGraphicsItemSetZValue(m_verticalLines, -255.f);
					EditorApi.qGraphicsItemSetZValue(m_horizonalLines, -255.f);

					m_viewRect = viewRect;
					m_gridStep = gridStep;
					m_color = color;
				}
			}
		}
	};
}

#endif