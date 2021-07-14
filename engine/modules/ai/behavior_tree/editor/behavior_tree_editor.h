#pragma once

#include "../behavior_tree.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class BehaviorTreeEditor : public ObjectEditor
	{
	public:
		BehaviorTreeEditor(Object* object);
		virtual ~BehaviorTreeEditor();

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
