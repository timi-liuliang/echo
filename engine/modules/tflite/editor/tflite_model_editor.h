#pragma once

#include "../tflite_model.h"
#include "engine/core/gizmos/Gizmos.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class TFLiteModelEditor : public ObjectEditor
	{
	public:
		TFLiteModelEditor(Object* object);
		virtual ~TFLiteModelEditor();

		// on editor select this node
		virtual void onEditorSelectThisNode() override;

		// post process
		virtual void postEditorCreateObject() override;

		// update self
		virtual void editor_update_self() override;

	private:
	
	};
#endif
}
