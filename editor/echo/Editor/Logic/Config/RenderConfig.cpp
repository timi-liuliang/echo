#include <engine/core/log/Log.h>
#include <engine/core/render/vulkan/vk.h>
#include <engine/core/render/metal/mt.h>
#include <engine/core/render/gles/GLES.h>

namespace Echo
{
	// initialize Renderer
	Renderer* initRender(size_t hwnd, int width, int height)
	{
		Renderer* renderer = nullptr;
#ifdef ECHO_PLATFORM_MAC
		LoadMTRenderer(renderer);
#elif defined ECHO_PLATFORM_HTML5
		LoadGLESRenderer(renderer);
#else
		LoadGLESRenderer(renderer);
		//LoadVKRenderer(renderer);
#endif

		Echo::Renderer::Config renderCfg;
		renderCfg.enableThreadedRendering = false;
		renderCfg.windowHandle = hwnd;

		EchoLogDebug("Canvas Size : %d x %d", renderCfg.screenWidth, renderCfg.screenHeight);
		if (renderer && renderer->initialize(renderCfg))
		{
			EchoLogInfo("Init %s Renderer success.", renderer->getName());
			return renderer;
		}

		EchoLogError("Root::initRenderer failed...");
		return renderer;
	}
}
