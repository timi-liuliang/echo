#include "../neuron.h"
#include "neuron_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	REGISTER_OBJECT_EDITOR(Neuron, NeuronEditor)

	NeuronEditor::NeuronEditor(Object* object)
		: ObjectEditor(object)
	{

	}

	// get camera2d icon, used for editor
	const char* NeuronEditor::getEditorIcon() const
	{
		return "engine/modules/ai/machine_learning/editor/icon/neuron.png";
	}
#endif
}