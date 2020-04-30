#include "text_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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

	ImagePtr UiTextEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/ui/editor/icon/text.png");
	}
#endif
}