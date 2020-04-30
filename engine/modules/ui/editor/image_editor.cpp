#include "image_editor.h"
#include "engine/core/editor/editor.h"
#include "engine/core/main/Engine.h"

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
	ImagePtr UiImageEditor::getThumbnail() const
	{
		return Image::loadFromFile(Engine::instance()->getRootPath() + "engine/modules/ui/editor/icon/image.png");
	}
#endif
}