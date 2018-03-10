#pragma once

#include <engine/core/render/render/Renderer.h>

namespace Echo
{
	// ĞÂ½¨GLESäÖÈ¾Æ÷
	void LoadGLESRenderer(Renderer*& render);

	// Ğ¶ÔØGLESäÖÈ¾Æ÷
	void UnLoadGLESRenderer(Renderer* render);
}