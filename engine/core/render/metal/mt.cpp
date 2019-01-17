#include "mt.h"
#include "mt_renderer.h"

namespace Echo
{
	void LoadMTRenderer(Renderer*& render)
	{
		render = EchoNew(MTRenderer);
	}

	void UnLoadMTRenderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}
