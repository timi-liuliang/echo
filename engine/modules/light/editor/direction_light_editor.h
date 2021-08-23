#pragma once

#include "../light/direction_light.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class DirectionLightEditor : public ObjectEditor
	{
	public:
		DirectionLightEditor(Object* object);
		virtual ~DirectionLightEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// post create this object
		virtual void postEditorCreateObject() override;

		// update self
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo = nullptr;
		TexturePtr	m_albedo;
	};
#endif
}
