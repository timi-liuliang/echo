#pragma once

#include <engine/core/render/base/renderer.h>

namespace Echo
{
	// load gles painter
	void LoadGLESRenderer(Renderer*& render);

	// unload gles painter
	void UnLoadGLESRenderer(Renderer* render);
}
