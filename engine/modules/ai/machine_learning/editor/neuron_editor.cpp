#include "../neuron.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	// get camera2d icon, used for editor
	const char* Neuron::getEditorIcon() const
	{
		return "engine/modules/ai/machine_learning/editor/icon/neuron.png";
	}
#endif
}