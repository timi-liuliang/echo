#pragma once

#include "../spline.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class SplinePointEditor : public ObjectEditor
	{
	public:
		SplinePointEditor(Object* object);
		virtual ~SplinePointEditor();

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
