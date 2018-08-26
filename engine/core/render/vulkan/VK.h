#pragma once

#include <engine/core/render/render/Renderer.h>

namespace Echo
{
	// ĞÂ½¨GLESäÖÈ¾Æ÷
	void LoadVKRenderer(Renderer*& render);

	// Ğ¶ÔØGLESäÖÈ¾Æ÷
	void UnLoadVKRenderer(Renderer* render);
}