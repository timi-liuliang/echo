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

		// get thumbnail, used for editor
		virtual ImagePtr getThumbnail() const override;

	private:
	};

#endif
}