#include <engine/core/log/Log.h>
#include <engine/core/render/vulkan/vk.h>
#include <engine/core/render/metal/mt.h>
#include <engine/core/render/gles/gles.h>
#include <engine/core/util/magic_enum.hpp>

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
		LoadVKRenderer(renderer);
#endif

		Echo::Renderer::Settings renderCfg;
		renderCfg.m_windowHandle = hwnd;

		EchoLogDebug("Canvas Size : %d x %d", renderCfg.m_windowWidth, renderCfg.m_windowHeight);
		if (renderer && renderer->initialize(renderCfg))
		{
            auto name = magic_enum::enum_name<Renderer::Type>(renderer->getType());
			EchoLogInfo("Init %s Renderer success.", std::string(name).c_str());

			return renderer;
		}

		EchoLogError("Root::initRenderer failed...");
		return renderer;
	}
}
