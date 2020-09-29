#pragma once

#include <engine/core/render/base/Renderer.h>

namespace Echo
{
	// load gles painter
	void LoadGLESRenderer(Renderer*& render);

	// unload gles painter
	void UnLoadGLESRenderer(Renderer* render);
}