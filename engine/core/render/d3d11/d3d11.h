#pragma once

#include <engine/core/render/base/renderer.h>

namespace Echo
{
	// new vulkan painter
	void LoadD3D11Renderer(Renderer*& render);

	// unload vk painter
	void UnLoadD3D11Renderer(Renderer* render);
}
