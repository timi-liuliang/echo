#include "GLES2.h"
#include "GLES2Render.h"

namespace LORD
{
	// ĞÂ½¨GLESäÖÈ¾Æ÷
	void LoadGLESRenderer(Renderer*& render)
	{
		render = LORD_NEW(GLES2Renderer);
	}

	// Ğ¶ÔØGLESäÖÈ¾Æ÷
	void UnLoadGLESRenderer(Renderer* render)
	{
		LordSafeDelete(render);
	}
}