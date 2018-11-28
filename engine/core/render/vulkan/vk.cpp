#include "vk.h"
#include "vk_renderer.h"

namespace Echo
{
	void LoadVKRenderer(Renderer*& render)
	{
		render = EchoNew(VKRenderer);
	}

	void UnLoadVKRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}
