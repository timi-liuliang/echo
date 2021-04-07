#pragma once

#include "engine/core/util/Array.hpp"
#include "engine/core/editor/editor_dock_panel.h"
#include "engine/modules/pcg/pcg_flow_graph.h"

#ifdef ECHO_EDITOR_MODE

#include "flowscene/qgraphics_flow_scene.h"
#include "ui_pcg_flow_graph_panel.h"

namespace Echo
{
	class PCGFlowGraphPanel : public QDockWidget, public Ui_PCGFlowGraphPanel
	{
		typedef vector<QGraphicsItem*>::type QGraphicsItemArray;
		typedef vector<QGraphicsProxyWidget*>::type QGraphicsWidgetArray;

	public:
		PCGFlowGraphPanel(Object* obj);
		virtual ~PCGFlowGraphPanel();

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
		QMenu*								m_menuNew = nullptr;
		Procedural::QGraphicsFlowScene*		m_graphicsScene = nullptr;
		Echo::Vector2						m_newPGNodePosition = Vector2::ZERO;
	};
}

#endif
