#pragma once

#include "../physx_body.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class PhysxControllerCapsuleEditor : public ObjectEditor
	{
	public:
		PhysxControllerCapsuleEditor(Object* object);
		virtual ~PhysxControllerCapsuleEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

	private:
	};
#endif
}
