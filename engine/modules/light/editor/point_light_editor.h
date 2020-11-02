#pragma once

#include "../point_light.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class PointLightEditor : public ObjectEditor
	{
	public:
		PointLightEditor(Object* object);
		virtual ~PointLightEditor();

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
