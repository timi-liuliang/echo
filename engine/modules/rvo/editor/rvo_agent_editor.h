#pragma once

#include "../rvo_agent.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class RvoAgentEditor : public ObjectEditor
	{
	public:
		RvoAgentEditor(Object* object);
		virtual ~RvoAgentEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// post process
		virtual void postEditorCreateObject() override;

		// update self
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo = nullptr;
	};
#endif
}
