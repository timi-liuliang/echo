#include "spline_control_point_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplineControlPointEditor::SplineControlPointEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	SplineControlPointEditor::~SplineControlPointEditor()
	{
	}

	ImagePtr SplineControlPointEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline_control_point.png");
	}

	void SplineControlPointEditor::onEditorSelectThisNode()
	{
	}

	void SplineControlPointEditor::editor_update_self()
	{

	}
#endif
}

