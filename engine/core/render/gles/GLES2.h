#pragma once

#include <engine/core/render/interface/Renderer.h>

namespace Echo
{
	// load gles painter
	void LoadGLESRenderer(Renderer*& render);

	// unload gles painter
	void UnLoadGLESRenderer(Renderer* render);
}