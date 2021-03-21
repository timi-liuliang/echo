#pragma once

#include <engine/core/editor/qt/QWidget.h>

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class QGraphicsSceneEx : public QGraphicsScene
	{
	protected:
	};
#endif
}
