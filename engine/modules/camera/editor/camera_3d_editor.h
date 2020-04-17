#include "../camera_3d.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/geom/Frustum.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class Camera3DEditor : public ObjectEditor
	{
	public:
		Camera3DEditor(Object* object);
		virtual ~Camera3DEditor();

		// get camera2d icon, used for editor
		virtual ImagePtr getThumbnail() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override { m_isSelect = true; }

		// on editor unselected this node
		virtual void onEditorUnSelectThisNode() override { m_isSelect = false; }

		// on editor update this
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo;
		bool		m_isSelect;
		Frustum		m_frustum;
	};
#endif
}
