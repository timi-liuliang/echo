#include "qgraphics_flow_scene.h"

#ifdef ECHO_EDITOR_MODE

namespace Procedural
{
	void QGraphicsFlowScene::beginConnect()
	{
		if (!m_editingConnectItem)
		{
			m_editingConnectItem = new QGraphicsConnectItem;
			m_editingConnectItem->set(Echo::Vector2(0.f, 0.f), Echo::Vector2(500.f, 500.f));

			addItem(m_editingConnectItem);
		}
	}

	void QGraphicsFlowScene::endConnect()
	{

	}
}

#endif