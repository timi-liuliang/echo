#include "GLES.h"
#include "gles_renderer.h"

namespace Echo
{
	void LoadGLESRenderer(Renderer*& render)
	{
		render = EchoNew(GLESRenderer);
	}

	void UnLoadGLESRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}