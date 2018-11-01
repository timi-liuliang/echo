#include "TimelinePanel.h"
#include "engine/core/editor/qt/QUiLoader.h"
#include "engine/core/editor/qt/QSplitter.h"

namespace Echo
{
	TimelinePanel::TimelinePanel()
	{
		m_ui = qLoadUi("engine/modules/anim/editor/TimelinePanel/TimelinePanel.ui");

		QWidget* splitter = qFindChild(m_ui, "m_splitter");
		if (splitter)
		{
			qSplitterSetStretchFactor(splitter, 0, 0);
			qSplitterSetStretchFactor(splitter, 1, 1);
		}
	}
}