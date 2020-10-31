#pragma once

#include "../spline.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class SplineSegmentEditor : public ObjectEditor
	{
	public:
		SplineSegmentEditor(Object* object);
		virtual ~SplineSegmentEditor();

		// get thumbnail
		virtual ImagePtr getThumbnail() const override;

		/* on editor selected this node*/
		virtual void onEditorSelectThisNode() override;

		// update self
		virtual void editor_update_self() override;

	private:
	};
#endif
}
