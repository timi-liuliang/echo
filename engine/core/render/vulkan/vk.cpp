#include "vk.h"
#include "vk_renderer.h"

namespace Echo
{
	// https://vulkan.lunarg.com/doc/view/1.2.162.1/mac/tutorial/html/index.html
	void LoadVKRenderer(Renderer*& render)
	{
		render = EchoNew(VKRenderer);
	}

	void UnLoadVKRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}
