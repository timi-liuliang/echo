#include "TimelinePanel.h"

namespace Echo
{
	// get title
	const char* TimelinePanel::getTitle() const
	{
		return "Timeline";
	}

	// get ui file
	const char* TimelinePanel::getUiFile() const
	{
		return "engine/modules/anim/editor/TimelinePanel/TimelinePanel.ui";
	}
}