#include "editor.h"
#include "engine/core/util/AssertX.h"

#ifdef ECHO_EDITOR_MODE
namespace Echo
{
	static Editor* g_editorInst = nullptr;

	// get instance
	Editor* Editor::instance()
	{
		EchoAssert(g_editorInst);
		return g_editorInst;
	}

	// set instance
	void Editor::setInstance(Editor* inst)
	{
		EchoAssert(!g_editorInst);
		g_editorInst = inst;
	}
}
#endif