#include "timeline_header_tool_bar.h"
#include "timeline_header.h"
#include "timeline_panel.h"
#include "engine/core/editor/editor.h"
#include "engine/modules/anim/anim_timeline.h"

#ifdef ECHO_EDITOR_MODE

#include "engine/core/main/Engine.h"

namespace Echo
{
	QTimelineHeaderToolBar::QTimelineHeaderToolBar(QTimelineHeader* parent)
		: QWidget(parent)
		, m_parent(parent)
	{
		setupUi(this);

		setToolbuttonIcon(m_addButton, "engine/modules/anim/editor/icon/add.png");
		setToolbuttonIcon(m_playButton, "engine/modules/anim/editor/icon/play.png");
		setToolbuttonIcon(m_stopButton, "engine/modules/anim/editor/icon/stop.png");
		setToolbuttonIcon(m_restartButton, "engine/modules/anim/editor/icon/replay.png");

		// Set fixed width of add toolbutton
		m_addButton->setIconSize(QSize(24, 24));

		// Connect signal slots
		EditorApi.qConnectWidget(m_addButton, QSIGNAL(clicked()), m_parent->getTimelinePanel(), createMethodBind(&TimelinePanel::onAddObject));
		EditorApi.qConnectWidget(m_playButton, QSIGNAL(clicked()), this, createMethodBind(&QTimelineHeaderToolBar::onPlayAnim));
		EditorApi.qConnectWidget(m_stopButton, QSIGNAL(clicked()), this, createMethodBind(&QTimelineHeaderToolBar::onStopAnim));
		EditorApi.qConnectWidget(m_restartButton, QSIGNAL(clicked()), this, createMethodBind(&QTimelineHeaderToolBar::onRestartAnim));
	}

	void QTimelineHeaderToolBar::setToolbuttonIcon(QToolButton* button, const String& iconPath)
	{
		if (button)
		{
			String fullPath = Engine::instance()->getRootPath() + iconPath;
			button->setIcon(QIcon(fullPath.c_str()));
		}
	}

	void QTimelineHeaderToolBar::onPlayAnim()
	{
		Timeline* timeline = m_parent->getTimelinePanel()->m_timeline;
		if (timeline)
		{
			if (timeline->getPlayState() != Timeline::PlayState::Playing)
			{
				timeline->play(m_parent->getTimelinePanel()->m_currentEditAnim.c_str());

				setToolbuttonIcon(m_playButton, "engine/modules/anim/editor/icon/pause.png");
			}
			else
			{
				timeline->pause();

				setToolbuttonIcon(m_playButton, "engine/modules/anim/editor/icon/play.png");
			}
		}
	}

	void QTimelineHeaderToolBar::onStopAnim()
	{
		Timeline* timeline = m_parent->getTimelinePanel()->m_timeline;
		if (timeline)
		{
			timeline->stop();

			// recover play button icon to "play.png"
			setToolbuttonIcon(m_playButton, "engine/modules/anim/editor/icon/play.png");
		}
	}

	void QTimelineHeaderToolBar::onRestartAnim()
	{
		onStopAnim();
		onPlayAnim();
	}
}

#endif