#include "d3d11.h"
#include "d3d11_renderer.h"

namespace Echo
{
	// https://vulkan.lunarg.com/doc/view/1.2.162.1/mac/tutorial/html/index.html
	void LoadD3D11Renderer(Renderer*& render)
	{
		render = EchoNew(VKRenderer);
	}

	void UnLoadD3D11Renderer(Renderer* render)
	{
		EchoSafeDelete(render, Renderer);
	}
}
