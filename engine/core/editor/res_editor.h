#pragma once

#include "object_editor.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	class ResEditor : public ObjectEditor
	{
	public:
		ResEditor(Object* object) : ObjectEditor(object) {}
		virtual ~ResEditor() {}

		// import
		virtual void onImport() = 0;
	};
}
#endif