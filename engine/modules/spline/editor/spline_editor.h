#pragma once

#include "../spline.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class SplineEditor : public ObjectEditor
	{
	public:
		SplineEditor(Object* object);
		virtual ~SplineEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// update self
		virtual void editor_update_self() override;

	private:
		Gizmos* m_gizmo = nullptr;
	};
#endif
}
