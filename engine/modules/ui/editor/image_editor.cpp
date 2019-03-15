#include "image_editor.h"
#include "engine/core/editor/editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	UiImageEditor::UiImageEditor(Object* object)
		: ObjectEditor(object)
	{
	}

	UiImageEditor::~UiImageEditor()
	{
	}

	// get camera2d icon, used for editor
	const char* UiImageEditor::getEditorIcon() const
	{
		return "engine/modules/ui/editor/icon/image.png";
	}
#endif
}