#include "../neural_network.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class NeuralLayerEditor : public ObjectEditor
	{
	public:
		NeuralLayerEditor(Object* object);

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;
	};
#endif
}