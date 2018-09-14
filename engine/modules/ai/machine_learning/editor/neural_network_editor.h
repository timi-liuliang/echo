#include "../neural_network.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class NeuralNetworkEditor : public ObjectEditor
	{
	public:
		NeuralNetworkEditor(Object* object);

		// get camera2d icon, used for editor
		const char* getEditorIcon() const;
	};
#endif
}