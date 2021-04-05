#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "engine/modules/pcg/pcg_flow_graph.h"

#ifdef ECHO_EDITOR_MODE

#include "flowscene/qgraphics_flow_scene.h"

namespace Echo
{
	class PCGFlowGraphPanel : public QObject
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		PCGFlowGraphPanel(Object* obj);
		virtual ~PCGFlowGraphPanel();

		// ui
		QDockWidget* getUi() { return m_ui; }

		// update
		void update();

	public:
		// new
		void onPlay();

		// import
		void onRightClickGraphicsView();
		void onNewPCGNode();

		// delete 
		void onDeletePGNodes();

	private:
		// add action
		void addActionToMenu(std::map<Echo::String, QMenu*>& subMenus, Echo::String& category, Echo::String& className);

	protected:
		PCGFlowGraph*						m_flowGraph = nullptr;
		QDockWidget*						m_ui;
		QMenu*								m_menuNew = nullptr;
		QGraphicsView*						m_graphicsView = nullptr;
		Procedural::QGraphicsFlowScene*		m_graphicsScene = nullptr;
		Echo::Vector2						m_newPGNodePosition = Vector2::ZERO;
	};
}

#endif
