#include "../curve.h"
#include "engine/core/gizmos/Gizmos.h"
#include "engine/core/geom/Frustum.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class CurveEditor : public ObjectEditor
	{
	public:
		CurveEditor(Object* object);
		virtual ~CurveEditor();

		// get camera2d icon, used for editor
		const char* getEditorIcon() const override;

		// on editor select this node
		virtual void onEditorSelectThisNode() override { m_isSelect = true; }

		// on editor unselect this node
		virtual void onEditorUnSelectThisNode() override { m_isSelect = false; }

		// on editor update this
		virtual void editor_update_self() override;

	private:
		Gizmos*		m_gizmo;
		bool		m_isSelect;
	};
#endif
}
