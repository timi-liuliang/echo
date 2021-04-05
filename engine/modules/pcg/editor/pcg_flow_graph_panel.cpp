#include "pcg_flow_graph_panel.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/editor/editor.h"
#include "engine/core/editor/qt/QWidgets.h"
#include "engine/core/base/class_method_bind.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/util/StringUtil.h"
#include "engine/core/io/memory_reader.h"
#include "engine/core/util/Buffer.h"
#include "engine/core/io/io.h"
#include "engine/core/render/base/image/image.h"
#include "engine/core/main/Engine.h"
#include "engine/core/render/base/atla/texture_atlas.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	PCGFlowGraphPanel::PCGFlowGraphPanel(Object* obj)
	{
		m_flowGraph = ECHO_DOWN_CAST<PCGFlowGraph*>(obj);

		m_ui = qobject_cast<QDockWidget*>(EditorApi.qLoadUi("engine/modules/pcg/editor/pcg_flow_graph_panel.ui"));

		QSplitter* splitter = m_ui->findChild<QSplitter*>("m_splitter");
		if (splitter)
		{
			splitter->setStretchFactor(0, 0);
			splitter->setStretchFactor(1, 1);
		}

		// Tool button icons
		m_ui->findChild<QToolButton*>("m_play")->setIcon(QIcon((Engine::instance()->getRootPath() + "engine/modules/pcg/editor/icon/play.png").c_str()));

		// connect signal slots
		EditorApi.qConnectWidget(m_ui->findChild<QWidget*>("m_graphicsView"), QSIGNAL(customContextMenuRequested(const QPoint&)), this, createMethodBind(&PCGFlowGraphPanel::onRightClickGraphicsView));
		EditorApi.qConnectWidget(m_ui->findChild<QWidget*>("m_play"), QSIGNAL(clicked()), this, createMethodBind(&PCGFlowGraphPanel::onPlay));
		EditorApi.qConnectAction(m_ui->findChild<QAction*>("m_actionDeleteNodes"), QSIGNAL(triggered()), this, createMethodBind(&PCGFlowGraphPanel::onDeletePGNodes));

		// create QGraphicsScene
		m_graphicsView = m_ui->findChild<QGraphicsView*>("m_graphicsView");
		m_graphicsView->setRenderHint(QPainter::Antialiasing);

		// graphics scene
		m_graphicsScene = new Procedural::QGraphicsFlowScene(m_graphicsView, m_flowGraph);
		m_graphicsView->setScene(m_graphicsScene);
	}

	PCGFlowGraphPanel::~PCGFlowGraphPanel()
	{
		EditorApi.removeCenterPanel(m_ui);
		delete m_ui; m_ui = nullptr;
	}

	void PCGFlowGraphPanel::update()
	{
		m_graphicsScene->update();
	}

	void PCGFlowGraphPanel::addActionToMenu(std::map<Echo::String, QMenu*>& subMenus, Echo::String& category, Echo::String& className)
	{
		Echo::String finalCategory = category.empty() ? "General" : category;

		QMenu* categoryMenu = nullptr;
		auto it = subMenus.find(category);
		if (it != subMenus.end())
		{
			categoryMenu = it->second;
		}

		if (!categoryMenu)
		{
			categoryMenu = new QMenu(m_ui);
			categoryMenu->setTitle(category.c_str());

			m_menuNew->addMenu(categoryMenu);
			subMenus[category] = categoryMenu;
		}

		QAction* newAction = new QAction;
		newAction->setText(Echo::StringUtil::Replace(className, "PCG", "").c_str());
		newAction->setData(className.c_str());
		categoryMenu->addAction(newAction);

		EditorApi.qConnectAction(newAction, QSIGNAL(triggered()), this, createMethodBind(&PCGFlowGraphPanel::onNewPCGNode));
	}

	void PCGFlowGraphPanel::onRightClickGraphicsView()
	{
		if (!m_menuNew)
		{
			m_menuNew = EchoNew(QMenu(m_ui));

			std::map<Echo::String, QMenu*> subMenus;
			Echo::StringArray pgNodeClasses;
			Echo::Class::getChildClasses(pgNodeClasses, "PCGNode", true);
			for (String& className : pgNodeClasses)
			{
				Echo::PCGNode* pcgNode = Echo::Class::create<PCGNode*>(className);
				Echo::String category = pcgNode->getCategory();
				EchoSafeDelete(pcgNode, PCGNode);

				addActionToMenu(subMenus, category, className);
			}
		}

		QPoint  localPos = m_graphicsView->mapFromGlobal(QCursor::pos());
		QPointF scenePos = m_graphicsView->mapToScene(localPos);
		m_newPGNodePosition = Echo::Vector2(scenePos.x(), scenePos.y());

		m_menuNew->exec(QCursor::pos());
	}

	void PCGFlowGraphPanel::onNewPCGNode()
	{
		QAction* action = qobject_cast<QAction*>(EditorApi.qSender());
		if(action)
		{ 
			Echo::String className = action->data().toString().toStdString().c_str();
			if (m_flowGraph)
			{
				Echo::PCGNode* pcgNode = Echo::Class::create<PCGNode*>(className);
				pcgNode->setPosition(m_newPGNodePosition);

				m_flowGraph->addNode(pcgNode);
			}
		}
	}

	void PCGFlowGraphPanel::onDeletePGNodes()
	{
		int a = 10;
	}

	void PCGFlowGraphPanel::onPlay()
	{
		if (m_flowGraph)
		{
			m_flowGraph->run();
		}
	}
}

#endif
