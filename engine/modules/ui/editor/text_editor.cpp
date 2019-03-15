#include "text_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	UiTextEditor::UiTextEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	UiTextEditor::~UiTextEditor()
	{
	}

	// get camera2d icon, used for editor
	const char* UiTextEditor::getEditorIcon() const
	{
		return "engine/modules/ui/editor/icon/text.png";
	}
#endif
}