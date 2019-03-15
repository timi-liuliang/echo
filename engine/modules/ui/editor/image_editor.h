#pragma once

#include "../base/image.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class UiImageEditor : public ObjectEditor
	{
	public:
		UiImageEditor(Object* object);
		virtual ~UiImageEditor();

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;

	private:
	};

#endif
}