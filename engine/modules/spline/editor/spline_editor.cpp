#include "spline_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	SplineEditor::SplineEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	SplineEditor::~SplineEditor()
	{
	}

	ImagePtr SplineEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/spline/editor/icon/spline.png");
	}

	void SplineEditor::onEditorSelectThisNode()
	{
	}

	void SplineEditor::editor_update_self()
	{

	}
#endif
}

