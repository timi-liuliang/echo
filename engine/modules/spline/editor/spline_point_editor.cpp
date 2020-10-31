#include "spline_point_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplinePointEditor::SplinePointEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	SplinePointEditor::~SplinePointEditor()
	{
	}

	ImagePtr SplinePointEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline_point.png");
	}

	void SplinePointEditor::editor_update_self()
	{

	}
#endif
}

