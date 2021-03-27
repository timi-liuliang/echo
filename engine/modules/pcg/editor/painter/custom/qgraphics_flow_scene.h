#pragma once

#include <engine/core/editor/qt/QWidget.h>

#ifdef ECHO_EDITOR_MODE

#include "qgraphics_connect_item.h"

namespace Procedural
{
	class QGraphicsFlowScene : public QGraphicsScene
	{
	public:
		// connection
		void beginConnect();
		void endConnect();

	protected:
		QGraphicsConnectItem*	m_editingConnectItem = nullptr;
	};
}

#endif
