#pragma once

#include <engine/core/render/base/Renderer.h>

namespace Echo
{
	// new vulkan painter
	void LoadVKRenderer(Renderer*& render);

	// unload vk painter
	void UnLoadVKRenderer(Renderer* render);
}