#include "TimelinePanel.h"
#include "engine/core/editor/qt/QUiLoader.h"
#include "engine/core/editor/qt/QSplitter.h"
#include "engine/core/editor/qt/QToolButton.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	TimelinePanel::TimelinePanel()
	{
		m_ui = qLoadUi("engine/modules/anim/editor/TimelinePanel/TimelinePanel.ui");

		QWidget* splitter = qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			qSplitterSetStretchFactor(splitter, 0, 0);
			qSplitterSetStretchFactor(splitter, 1, 1);
		}

		// Top toolbuttons icons
		qToolButtonSetIcon(qFindChild(m_ui, "AddNode"), "engine/modules/anim/editor/icon/new.png");
		qToolButtonSetIcon(qFindChild(m_ui, "Play"), "engine/modules/anim/editor/icon/new.png");
		qToolButtonSetIcon(qFindChild(m_ui, "Stop"), "engine/modules/anim/editor/icon/new.png");
		qToolButtonSetIcon(qFindChild(m_ui, "Restart"), "engine/modules/anim/editor/icon/new.png");

		// set toolbuttons icons
		qToolButtonSetIcon( qFindChild(m_ui, "NewClip"), "engine/modules/anim/editor/icon/new.png");
		qToolButtonSetIcon( qFindChild(m_ui, "DuplicateClip"), "engine/modules/anim/editor/icon/duplicate.png");
		qToolButtonSetIcon( qFindChild(m_ui, "DeleteClip"), "engine/modules/anim/editor/icon/delete.png");
	}
#endif
}