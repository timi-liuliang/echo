#include "GLES2.h"
#include "GLES2Render.h"

namespace Echo
{
	// ĞÂ½¨GLESäÖÈ¾Æ÷
	void LoadGLESRenderer(Renderer*& render)
	{
		render = EchoNew(GLES2Renderer);
	}

	// Ğ¶ÔØGLESäÖÈ¾Æ÷
	void UnLoadGLESRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}