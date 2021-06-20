#pragma once

#include "../physx_body.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class PhysxControllerBoxEditor : public ObjectEditor
	{
	public:
		PhysxControllerBoxEditor(Object* object);
		virtual ~PhysxControllerBoxEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// update self
		virtual void editor_update_self() override;

	private:
	};
#endif
}
