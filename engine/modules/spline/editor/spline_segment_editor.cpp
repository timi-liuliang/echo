#include "spline_segment_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplineSegmentEditor::SplineSegmentEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	SplineSegmentEditor::~SplineSegmentEditor()
	{
	}

	ImagePtr SplineSegmentEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline_segment.png");
	}

	void SplineSegmentEditor::onEditorSelectThisNode()
	{
	}

	void SplineSegmentEditor::editor_update_self()
	{

	}
#endif
}

