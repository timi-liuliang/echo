#include "timeline_header_tool_bar.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/main/Engine.h"

namespace Echo
{
	QTimelineHeaderToolBar::QTimelineHeaderToolBar(QWidget* parent)
		: QWidget(parent)
	{
		setupUi(this);

		setToolbuttonIcon(m_addButton, "engine/modules/anim/editor/icon/add.png");
		setToolbuttonIcon(m_playButton, "engine/modules/anim/editor/icon/play.png");
		setToolbuttonIcon(m_stopButton, "engine/modules/anim/editor/icon/stop.png");
		setToolbuttonIcon(m_restartButton, "engine/modules/anim/editor/icon/replay.png");

		// set fixed width of add toolbutton
		m_addButton->setIconSize(QSize(24, 24));
	}

	void QTimelineHeaderToolBar::setToolbuttonIcon(QToolButton* button, const String& iconPath)
	{
		if (button)
		{
			String fullPath = Engine::instance()->getRootPath() + iconPath;
			button->setIcon(QIcon(fullPath.c_str()));
		}
	}
}

#endif