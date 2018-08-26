#include "vk.h"
#include "vk_render.h"

namespace Echo
{
	void LoadGLESRenderer(Renderer*& render)
	{
		render = nullptr;// EchoNew(GLES2Renderer);
	}

	void UnLoadGLESRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}