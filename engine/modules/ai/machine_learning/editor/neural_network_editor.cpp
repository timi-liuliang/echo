#include "../neural_network.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// get camera2d icon, used for editor
	const char* NeuralNetwork::getEditorIcon() const
	{
		return "engine/modules/ai/machine_learning/editor/icon/neuralnetwork.png";
	}

	// on editor select this node
	void NeuralNetwork::onEditorSelectThisNode()
	{

	}
#endif
}