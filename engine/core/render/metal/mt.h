#pragma once

#include <engine/core/render/interface/Renderer.h>

namespace Echo
{
	// new vulkan painter
	void LoadMTRenderer(Renderer*& render);

	// unload vk painter
	void UnLoadMTRenderer(Renderer* render);
}
