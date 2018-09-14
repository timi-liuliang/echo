#include "neural_network_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	REGISTER_OBJECT_EDITOR(NeuralNetwork, NeuralNetworkEditor)

	NeuralNetworkEditor::NeuralNetworkEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* NeuralNetworkEditor::getEditorIcon() const
	{
		return "engine/modules/ai/machine_learning/editor/icon/neuralnetwork.png";
	}
#endif
}