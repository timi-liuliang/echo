#pragma once

#include "../cube_light_capture.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class CubeLightCaptureEditor : public ObjectEditor
	{
	public:
		CubeLightCaptureEditor(Object* object);
		virtual ~CubeLightCaptureEditor();

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
