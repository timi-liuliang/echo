#include "neural_layer_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	NeuralLayerEditor::NeuralLayerEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* NeuralLayerEditor::getEditorIcon() const
	{
		return "engine/modules/ai/machine_learning/editor/icon/neurallayer.png";
	}
#endif
}