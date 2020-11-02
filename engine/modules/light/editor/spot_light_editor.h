#pragma once

#include "../spot_light.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class SpotLightEditor : public ObjectEditor
	{
	public:
		SpotLightEditor(Object* object);
		virtual ~SpotLightEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// update self
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo = nullptr;
		TexturePtr	m_albedo;
	};
#endif
}
