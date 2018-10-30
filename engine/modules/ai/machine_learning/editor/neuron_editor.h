#include "../neuron.h"
#include "engine/core/editor/object_editor.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class NeuronEditor : public ObjectEditor
	{
	public:
		NeuronEditor(Object* object);

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;
	};
#endif
}