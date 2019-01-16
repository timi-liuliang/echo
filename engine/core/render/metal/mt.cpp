#include "mt.h"
#include "mt_renderer.h"

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
