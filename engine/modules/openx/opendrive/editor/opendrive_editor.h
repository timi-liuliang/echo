#pragma once

#include "../opendrive.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class OpenDriveEditor : public ObjectEditor
	{
	public:
		OpenDriveEditor(Object* object);
		virtual ~OpenDriveEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// post process
		virtual void postEditorCreateObject() override;

		// update self
		virtual void editor_update_self() override;

	private:
	
	};
#endif
}
