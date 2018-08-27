#include "vk.h"

namespace Echo
{
	void LoadVKRenderer(Renderer*& render)
	{
		render = nullptr;// EchoNew(GLES2Renderer);
	}

	void UnLoadVKRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}