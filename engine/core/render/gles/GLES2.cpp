#include "GLES2.h"
#include "GLES2Render.h"

namespace Echo
{
	void LoadGLESRenderer(Renderer*& render)
	{
		render = EchoNew(GLES2Renderer);
	}

	void UnLoadGLESRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}